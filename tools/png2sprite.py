#!/usr/bin/env python3
import sys
import json
import argparse
import math
import colorsys
from PIL import Image, ImageFilter

# Amstrad CPC full hardware palette (27 colors)
# Mapping: (R, G, B) -> (character, index, CPCTelera_name)
CPC_PALETTE = {
    (  0,   0,   0): ('.',  0, 'HW_BLACK'),
    (  0,   0, 128): ('S',  1, 'HW_BLUE'),
    (  0,   0, 255): ('B',  2, 'HW_BRIGHT_BLUE'),
    (128,   0,   0): ('r',  3, 'HW_RED'),
    (128,   0, 128): ('u',  4, 'HW_MAGENTA'),
    (128,   0, 255): ('m',  5, 'HW_MAUVE'),
    (255,   0,   0): ('R',  6, 'HW_BRIGHT_RED'),
    (255,   0, 128): ('p',  7, 'HW_PURPLE'),
    (255,   0, 255): ('M',  8, 'HW_BRIGHT_MAGENTA'),
    (  0, 128,   0): ('g',  9, 'HW_GREEN'),
    (  0, 128, 128): ('c', 10, 'HW_CYAN'),
    (  0, 128, 255): ('s', 11, 'HW_SKY_BLUE'),
    (128, 128,   0): ('l', 12, 'HW_YELLOW'),
    (128, 128, 128): ('w', 13, 'HW_WHITE'),
    (128, 128, 255): ('b', 14, 'HW_PASTEL_BLUE'),
    (255, 128,   0): ('o', 15, 'HW_ORANGE'),
    (255, 128, 128): ('k', 16, 'HW_PINK'),
    (255, 128, 255): ('q', 17, 'HW_PASTEL_MAGENTA'),
    (  0, 255,   0): ('G', 18, 'HW_BRIGHT_GREEN'),
    (  0, 255, 128): ('e', 19, 'HW_SEA_GREEN'),
    (  0, 255, 255): ('C', 20, 'HW_BRIGHT_CYAN'),
    (128, 255,   0): ('i', 21, 'HW_LIME'),
    (128, 255, 128): ('n', 22, 'HW_PASTEL_GREEN'),
    (128, 255, 255): ('t', 23, 'HW_PASTEL_CYAN'),
    (255, 255,   0): ('Y', 24, 'HW_BRIGHT_YELLOW'),
    (255, 255, 128): ('d', 25, 'HW_PASTEL_YELLOW'),
    (255, 255, 255): ('W', 26, 'HW_BRIGHT_WHITE'),
}

def rgb_to_lab(rgb):
    """
    Very basic RGB to CIELab conversion. 
    Assumes sRGB and D65 white point.
    """
    # 1. Normalize to [0, 1]
    res = [c / 255.0 for c in rgb]

    # 2. Linearize RGB
    for i in range(3):
        if res[i] <= 0.04045:
            res[i] /= 12.92
        else:
            res[i] = ((res[i] + 0.055) / 1.055) ** 2.4

    # 3. Linear RGB to XYZ (D65)
    x = res[0] * 0.4124 + res[1] * 0.3576 + res[2] * 0.1805
    y = res[0] * 0.2126 + res[1] * 0.7152 + res[2] * 0.0722
    z = res[0] * 0.0193 + res[1] * 0.1192 + res[2] * 0.9505

    # 4. XYZ to Lab
    # White point D65
    xw, yw, zw = 0.95047, 1.00000, 1.08883
    
    def f(t):
        if t > 0.008856:
            return t ** (1/3)
        else:
            return (7.787 * t) + (16 / 116)

    fx = f(x / xw)
    fy = f(y / yw)
    fz = f(z / zw)

    L = (116 * fy) - 16
    a = 500 * (fx - fy)
    b = 200 * (fy - fz)
    
    return (L, a, b)

def rgb_to_hsv_cartesian(rgb):
    """
    Convert RGB to a Cartesian representation of HSV.
    This avoids wrap-around issues with Hue when calculating distance.
    s * cos(h*2pi) * 255, s * sin(h*2pi) * 255, v * 255
    """
    r, g, b = [x / 255.0 for x in rgb]
    h, s, v = colorsys.rgb_to_hsv(r, g, b)
    x = s * math.cos(h * 2 * math.pi) * 255.0
    y = s * math.sin(h * 2 * math.pi) * 255.0
    z = v * 255.0
    return (x, y, z)

def get_distance_func(metric_name, weights=None):
    if weights is None:
        if metric_name == 'manhattan':
            return lambda c1, c2: sum(abs(c1[i] - c2[i]) for i in range(len(c1)))
        elif metric_name == 'euclidean':
            return lambda c1, c2: math.sqrt(sum((c1[i] - c2[i])**2 for i in range(len(c1))))
        return lambda c1, c2: sum(abs(c1[i] - c2[i]) for i in range(len(c1)))
    else:
        if metric_name == 'manhattan':
            return lambda c1, c2: sum(weights[i] * abs(c1[i] - c2[i]) for i in range(len(c1)))
        elif metric_name == 'euclidean':
            return lambda c1, c2: math.sqrt(sum(weights[i] * (c1[i] - c2[i])**2 for i in range(len(c1))))
        return lambda c1, c2: sum(weights[i] * abs(c1[i] - c2[i]) for i in range(len(c1)))

def get_closest_color_info(pixel_val, palette_subset_values, distance_func, neighbor_cpc_rgbs=None, use_hysteresis=False):
    """
    pixel_val: The value of the pixel in the target space (RGB or Lab)
    palette_subset_values: List of (pixel_val_in_space, cpc_rgb, info)
    distance_func: function taking two tuples
    neighbor_cpc_rgbs: List of previously selected cpc_rgb values for adjacent pixels
    use_hysteresis: boolean whether to apply distance discount for neighbors
    """
    best_info = None
    min_dist = float('inf')
    
    for palette_val, cpc_rgb, info in palette_subset_values:
        dist = distance_func(pixel_val, palette_val)
        
        # Apply hysteresis: discount distance if this color is the same as a neighbor's
        if use_hysteresis and neighbor_cpc_rgbs and cpc_rgb in neighbor_cpc_rgbs:
            dist *= 0.85 # 15% discount
            
        if dist < min_dist:
            min_dist = dist
            best_info = (cpc_rgb, info)
    return best_info

def get_optimal_16_palette(img, space, distance_func, excluded_indices=None):
    width, height = img.size
    if excluded_indices is None:
        excluded_indices = []

    # Pre-calculate CPC palette in target space, filtering out excluded indices
    cpc_in_space = {} # CPC_RGB -> value_in_space
    for rgb, info in CPC_PALETTE.items():
        if info[1] in excluded_indices:
            continue
        if space == 'lab':
            cpc_in_space[rgb] = rgb_to_lab(rgb)
        elif space == 'hsv':
            cpc_in_space[rgb] = rgb_to_hsv_cartesian(rgb)
        else:
            cpc_in_space[rgb] = rgb
            
    # Pre-calculate best match for every pixel in the image among the available colors
    # and store which original pixels map to which CPC color
    cpc_to_original_pixels_in_space = {color: [] for color in cpc_in_space.keys()}
    
    for y in range(height):
        for x in range(width):
            pixel_rgb = img.getpixel((x, y))
            if space == 'lab':
                pixel_val = rgb_to_lab(pixel_rgb)
            elif space == 'hsv':
                pixel_val = rgb_to_hsv_cartesian(pixel_rgb)
            else:
                pixel_val = pixel_rgb
            
            # Get closest CPC color (RGB) from the available set
            best_cpc_rgb = None
            min_dist = float('inf')
            for cpc_rgb, cpc_val in cpc_in_space.items():
                dist = distance_func(pixel_val, cpc_val)
                if dist < min_dist:
                    min_dist = dist
                    best_cpc_rgb = cpc_rgb
            if best_cpc_rgb is not None:
                cpc_to_original_pixels_in_space[best_cpc_rgb].append(pixel_val)

    # Filter out colors from our available set that aren't even used as a best match
    current_palette_rgbs = [rgb for rgb, pixels in cpc_to_original_pixels_in_space.items() if len(pixels) > 0]
    
    # 2. Iteratively remove colors until 16 remain
    while len(current_palette_rgbs) > 16:
        best_removal_rgb = None
        min_total_penalty = float('inf')
        
        for candidate_rgb in current_palette_rgbs:
            # Skip protecting HW_BLACK (0,0,0) if possible
            if candidate_rgb == (0, 0, 0): continue
            
            # Penalty is the cost of reassigning all pixels of 'candidate_rgb' 
            # to the NEXT best available color in the remaining palette
            remaining_palette_vals = [cpc_in_space[rgb] for rgb in current_palette_rgbs if rgb != candidate_rgb]
            candidate_val = cpc_in_space[candidate_rgb]
            
            total_penalty = 0
            for p_val in cpc_to_original_pixels_in_space[candidate_rgb]:
                # Find closest color in remaining_palette
                local_min_dist = min(distance_func(p_val, r_val) for r_val in remaining_palette_vals)
                # The "penalty" is how much worse this pixel becomes
                current_dist = distance_func(p_val, candidate_val)
                total_penalty += (local_min_dist - current_dist)
            
            if total_penalty < min_total_penalty:
                min_total_penalty = total_penalty
                best_removal_rgb = candidate_rgb
        
        if best_removal_rgb is None: # Should not happen unless all are black or all are excluded
            break
            
        current_palette_rgbs.remove(best_removal_rgb)

    # Prepare formatted subset for get_closest_color_info
    # List of (val_in_space, cpc_rgb, info)
    subset_info = []
    for rgb in current_palette_rgbs:
        subset_info.append((cpc_in_space[rgb], rgb, CPC_PALETTE[rgb]))
        
    return subset_info

def add_error_to_pixel(img_array, x, y, width, height, err_r, err_g, err_b, factor):
    if 0 <= x < width and 0 <= y < height:
        r, g, b = img_array[y][x]
        img_array[y][x] = (
            max(0, min(255, int(r + err_r * factor))),
            max(0, min(255, int(g + err_g * factor))),
            max(0, min(255, int(b + err_b * factor)))
        )

def apply_bilateral(img, spatial_sigma=2.0, color_sigma=20.0):
    """
    Basic edge-preserving bilateral filter.
    """
    width, height = img.size
    new_img = img.copy()
    pixels = img.load()
    new_pixels = new_img.load()
    
    size = 2 # 5x5 kernel
    for y in range(height):
        for x in range(width):
            center_rgb = pixels[x, y]
            total_weight = 0
            res_r, res_g, res_b = 0, 0, 0
            
            for j in range(-size, size + 1):
                for i in range(-size, size + 1):
                    nx, ny = x + i, y + j
                    if 0 <= nx < width and 0 <= ny < height:
                        neighbor_rgb = pixels[nx, ny]
                        
                        # Spatial weight: e^(- (dist^2) / (2 * sigma_s^2))
                        dist_sq = i*i + j*j
                        w_spatial = math.exp(-dist_sq / (2 * spatial_sigma**2))
                        
                        # Color weight: e^(- (color_dist^2) / (2 * sigma_c^2))
                        color_dist_sq = sum((center_rgb[k] - neighbor_rgb[k])**2 for k in range(3))
                        w_color = math.exp(-color_dist_sq / (2 * color_sigma**2))
                        
                        w = w_spatial * w_color
                        res_r += neighbor_rgb[0] * w
                        res_g += neighbor_rgb[1] * w
                        res_b += neighbor_rgb[2] * w
                        total_weight += w
            
            new_pixels[x, y] = (int(res_r / total_weight), int(res_g / total_weight), int(res_b / total_weight))
    return new_img

def convert_png(args):
    filename = args.filename
    try:
        img = Image.open(filename).convert('RGB')
    except Exception as e:
        print(f"Error opening {filename}: {e}")
        return

    # Resize before any other processing if requested
    if args.resize:
        new_w, new_h = args.resize
        img = img.resize((new_w, new_h), Image.NEAREST)
        print(f"# Resized image to {new_w}x{new_h} (Nearest Neighbor)")

    width, height = img.size
    total_pixels = width * height

    # 0. Apply Black Preservation if requested
    if args.preserve_black is not None:
        threshold = args.preserve_black
        pixels = img.load()
        for y in range(height):
            for x in range(width):
                r, g, b = pixels[x, y]
                # Luminance formula (standard BT.601)
                luminance = 0.299*r + 0.587*g + 0.114*b
                if luminance < threshold:
                    pixels[x, y] = (0, 0, 0)
        print(f"# Appled Black Preservation (Threshold: {threshold})")

    # 1. Apply pre-quantization filters if selected
    if args.filter == 'median':
        img = img.filter(ImageFilter.MedianFilter(size=3))
        print("# Applied Median Filter (size 3).")
    elif args.filter == 'bilateral':
        img = apply_bilateral(img)
        print("# Applied Bilateral Filter (edge-preserving).")

    
    distance_func = get_distance_func(args.distance, args.weights)
    
    # Get optimal 16-color palette, respecting excluded list
    optimal_subset = get_optimal_16_palette(img, args.space, distance_func, excluded_indices=args.exclude)
    
    base_out = args.out if args.out else filename.rsplit('.', 1)[0]
    if args.out and base_out.lower().endswith(('.png', '.json', '.txt', '.c', '.h')):
        base_out = base_out.rsplit('.', 1)[0]

    sprite_filename = base_out + '_sprite.txt'
    
    print(f"# Sprite converted from {filename} ({width}x{height})")
    print(f"# Optimized to {len(optimal_subset)} colors using {args.distance} distance in {args.space} space")
    if args.exclude:
        print(f"# Excluded CPC indices: {args.exclude}")
    if args.filter != 'none':
        print(f"# Applied spatial filter: {args.filter}")
    
    sprite_rows = []
    
    usage_counts = {} # cpc_index -> count
    for _, _, info in optimal_subset:
        usage_counts[info[1]] = 0

    # For Hysteresis, we need to track pixel choices
    chosen_cpc_rgbs = [[None for _ in range(width)] for _ in range(height)]
    
    # For Dithering, we need a float representation of the image to accumulate error
    dither_array = [[img.getpixel((x, y)) for x in range(width)] for y in range(height)]

    for y in range(height):
        row = ""
        for x in range(width):
            pixel_rgb = dither_array[y][x] if args.filter == 'dither' else img.getpixel((x, y))
            if args.space == 'lab':
                pixel_val = rgb_to_lab(pixel_rgb)
            elif args.space == 'hsv':
                pixel_val = rgb_to_hsv_cartesian(pixel_rgb)
            else:
                pixel_val = pixel_rgb
            
            neighbor_rgbs = []
            if args.filter == 'hysteresis':
                if x > 0 and chosen_cpc_rgbs[y][x-1] is not None:
                    neighbor_rgbs.append(chosen_cpc_rgbs[y][x-1])
                if y > 0 and chosen_cpc_rgbs[y-1][x] is not None:
                    neighbor_rgbs.append(chosen_cpc_rgbs[y-1][x])

            cpc_rgb, info = get_closest_color_info(
                pixel_val, 
                optimal_subset, 
                distance_func, 
                neighbor_cpc_rgbs=neighbor_rgbs, 
                use_hysteresis=(args.filter == 'hysteresis')
            )
            
            chosen_cpc_rgbs[y][x] = cpc_rgb
            row += info[0]
            usage_counts[info[1]] += 1
            
            # Apply Floyd-Steinberg dithering error to neighbors
            if args.filter == 'dither':
                err_r = pixel_rgb[0] - cpc_rgb[0]
                err_g = pixel_rgb[1] - cpc_rgb[1]
                err_b = pixel_rgb[2] - cpc_rgb[2]
                
                add_error_to_pixel(dither_array, x + 1, y,     width, height, err_r, err_g, err_b, 7/16)
                add_error_to_pixel(dither_array, x - 1, y + 1, width, height, err_r, err_g, err_b, 3/16)
                add_error_to_pixel(dither_array, x,     y + 1, width, height, err_r, err_g, err_b, 5/16)
                add_error_to_pixel(dither_array, x + 1, y + 1, width, height, err_r, err_g, err_b, 1/16)
            
        sprite_rows.append(row)

    # Write sprite data and stats to file
    try:
        with open(sprite_filename, 'w') as f_out:
            f_out.write(f"# Sprite converted from {filename} ({width}x{height})\n")
            f_out.write(f"# Optimized to {len(optimal_subset)} colors using {args.distance} distance in {args.space} space\n")
            if args.exclude:
                f_out.write(f"# Excluded CPC indices: {args.exclude}\n")
            if args.filter != 'none':
                f_out.write(f"# Applied spatial filter: {args.filter}\n")
            
            f_out.write("sprite_data = [\n")
            for i, row in enumerate(sprite_rows):
                comma = "," if i < len(sprite_rows) - 1 else ""
                f_out.write(f'    "{row}"{comma}\n')
            f_out.write("]\n\n")
            
            f_out.write("# Color Usage Statistics:\n")
            for _, _, info in optimal_subset:
                count = usage_counts[info[1]]
                percentage = (count / total_pixels) * 100
                f_out.write(f"#   {info[2]:<20} (Index {info[1]:>2}): {count:>6} pixels ({percentage:>6.2f}%)\n")
        
        print(f"# Sprite data and statistics saved to {sprite_filename}")
    except Exception as e:
        print(f"# Error saving sprite file: {e}")

    # Create and save an optimized preview image
    preview_img = Image.new('RGB', (width, height))
    total_squared_error = 0

    # `img` is the original image (after possible black preservation/spatial filters but before quantization)
    original_pixels = img.load() 
    for y in range(height):
        for x in range(width):
            chosen_rgb = chosen_cpc_rgbs[y][x]
            preview_img.putpixel((x, y), chosen_rgb)
            
            orig_r, orig_g, orig_b = original_pixels[x, y]
            total_squared_error += (orig_r - chosen_rgb[0])**2 + (orig_g - chosen_rgb[1])**2 + (orig_b - chosen_rgb[2])**2

    mse = total_squared_error / (width * height * 3)
    print(f"# Conversion Loss (RGB MSE): {mse:.2f}")

    preview_filename = base_out + '_optimized.png'
    try:
        preview_img.save(preview_filename)
        print(f"# Optimized preview saved to {preview_filename}")
    except Exception as e:
        print(f"# Error saving preview image: {e}")

    # Export the used palette to a JSON file
    json_filename = base_out + '_palette.json'
    
    palette_entries = []
    for val_in_space, rgb, info in optimal_subset:
        count = usage_counts[info[1]]
        percentage = (count / total_pixels) * 100
        palette_entries.append({
            "cpc_rgb": list(rgb),
            "cpc_hex": f"#{rgb[0]:02x}{rgb[1]:02x}{rgb[2]:02x}",
            "mapped_char": info[0],
            "cpc_index": info[1],
            "cpc_hw_name": info[2],
            "pixel_count": count,
            "usage_percentage": round(percentage, 2)
        })

    # Sort by CPC index
    palette_entries.sort(key=lambda x: x["cpc_index"])

    palette_output = {
        "source_file": filename,
        "config": {
            "distance_metric": args.distance,
            "weights": args.weights,
            "color_space": args.space,
            "excluded_indices": args.exclude if args.exclude else [],
            "filter": args.filter,
            "preserve_black_threshold": args.preserve_black
        },
        "total_pixels": total_pixels,
        "colors_used_count": len(palette_entries),
        "palette": palette_entries
    }
        
    try:
        with open(json_filename, 'w') as f:
            json.dump(palette_output, f, indent=4)
        print(f"# Used palette ({len(palette_entries)} colors) saved to {json_filename}")
    except Exception as e:
        print(f"# Error saving palette json: {e}")

def list_cpc_palette():
    print("# Amstrad CPC Hardware Palette (27 Colors):")
    print(f"{'Index':>5} | {'Color':^7} | {'CPCTelera Name':<20} | {'RGB Value':<15} | {'HEX':<7}")
    print("-" * 68)
    # Sort by CPC index
    sorted_palette = sorted(CPC_PALETTE.items(), key=lambda x: x[1][1])
    for rgb, info in sorted_palette:
        char, idx, name = info
        hex_val = f"#{rgb[0]:02x}{rgb[1]:02x}{rgb[2]:02x}"
        # ANSI escape code for background color: \033[48;2;r;g;bm  \033[0m
        ansi_color = f"\033[48;2;{rgb[0]};{rgb[1]};{rgb[2]}m  \033[0m"
        print(f"{idx:>5} |  {ansi_color}   | {name:<20} | {str(rgb):<15} | {hex_val:<7}")

def show_image_info(filename):
    try:
        img = Image.open(filename).convert('RGB')
    except Exception as e:
        print(f"Error opening {filename}: {e}")
        return

    width, height = img.size
    colors = img.getdata()
    unique_colors = set(colors)
    
    print(f"# Image Information: {filename}")
    print(f"#   Dimensions: {width}x{height}")
    print(f"#   Unique Colors: {len(unique_colors)}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Convert PNG to Amstrad CPC Mode 0 sprite.')
    parser.add_argument('filename', nargs='?', help='Path to the PNG file')
    parser.add_argument('--distance', choices=['manhattan', 'euclidean'], default='euclidean', help='Color distance metric (default: manhattan)')
    parser.add_argument('--weights', type=float, nargs=3, metavar=('W1', 'W2', 'W3'), help='Three weights for the color components (e.g., 3 4 2 for RGB to prioritize green)')
    parser.add_argument('--space', choices=['rgb', 'lab', 'hsv'], default='lab', help='Color space to use (default: rgb)')
    parser.add_argument('--exclude', type=int, nargs='+', help='List of CPC hardware indices to exclude from optimization')
    parser.add_argument('--filter', choices=['none', 'median', 'bilateral', 'hysteresis', 'dither'], default='none', help='Spatial filter to stabilize colors (default: none)')
    parser.add_argument('--preserve-black', type=int, metavar='THRESHOLD', help='Force pixels darker than THRESHOLD (0-255) to pure black before conversion to recover outlines')
    parser.add_argument('--resize', type=int, nargs=2, metavar=('WIDTH', 'HEIGHT'), help='Resize the image to WIDTHxHEIGHT using Nearest Neighbor (no subsampling)')
    parser.add_argument('--out', type=str, help='Base name for output files (optional)')
    parser.add_argument('--list-palette', action='store_true', help='List all 27 Amstrad CPC hardware colors and exit')
    parser.add_argument('--info', action='store_true', help='Show image dimensions and number of unique colors')
    
    args = parser.parse_args()
    
    if args.list_palette:
        list_cpc_palette()
        sys.exit(0)
        
    if not args.filename:
        parser.print_help()
        sys.exit(1)
        
    if args.info:
        show_image_info(args.filename)
        sys.exit(0)
        
    convert_png(args)
