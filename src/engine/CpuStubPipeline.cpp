#include "engine/CpuStubPipeline.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace lumos::engine {

namespace {

struct Pixel {
    int r {0};
    int g {0};
    int b {0};
};

struct Image {
    int width {0};
    int height {0};
    int max_value {255};
    std::vector<Pixel> pixels;
};

bool parsePpm(const std::string& path, Image* image, std::string* error_message) {
    std::ifstream input(path, std::ios::in);
    if (!input.good()) {
        if (error_message != nullptr) {
            *error_message = "failed to open file";
        }
        return false;
    }

    std::vector<std::string> tokens;
    std::string token;
    while (input >> token) {
        if (!token.empty() && token[0] == '#') {
            std::string skip_line;
            std::getline(input, skip_line);
            continue;
        }
        tokens.push_back(token);
    }

    if (tokens.size() < 4 || tokens[0] != "P3") {
        if (error_message != nullptr) {
            *error_message = "unsupported or invalid ppm header";
        }
        return false;
    }

    int width = 0;
    int height = 0;
    int max_value = 0;

    try {
        width = std::stoi(tokens[1]);
        height = std::stoi(tokens[2]);
        max_value = std::stoi(tokens[3]);
    } catch (...) {
        if (error_message != nullptr) {
            *error_message = "invalid ppm dimensions";
        }
        return false;
    }

    if (width <= 0 || height <= 0 || max_value <= 0) {
        if (error_message != nullptr) {
            *error_message = "ppm dimensions must be positive";
        }
        return false;
    }

    const std::size_t expected_channel_count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 3;
    if (tokens.size() < 4 + expected_channel_count) {
        if (error_message != nullptr) {
            *error_message = "ppm data is incomplete";
        }
        return false;
    }

    Image parsed {
        .width = width,
        .height = height,
        .max_value = max_value,
        .pixels = std::vector<Pixel>(static_cast<std::size_t>(width) * static_cast<std::size_t>(height)),
    };

    std::size_t index = 4;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Pixel pixel {};
            try {
                pixel.r = std::clamp(std::stoi(tokens[index++]), 0, max_value);
                pixel.g = std::clamp(std::stoi(tokens[index++]), 0, max_value);
                pixel.b = std::clamp(std::stoi(tokens[index++]), 0, max_value);
            } catch (...) {
                if (error_message != nullptr) {
                    *error_message = "ppm pixel parse failed";
                }
                return false;
            }
            parsed.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(width) + static_cast<std::size_t>(x)] =
                pixel;
        }
    }

    if (image != nullptr) {
        *image = std::move(parsed);
    }
    return true;
}

bool writePpm(const Image& image, const std::string& path, std::string* error_message) {
    const std::filesystem::path output_path(path);
    const auto output_dir = output_path.parent_path();
    if (!output_dir.empty()) {
        std::filesystem::create_directories(output_dir);
    }

    std::ofstream output(path, std::ios::out | std::ios::trunc);
    if (!output.good()) {
        if (error_message != nullptr) {
            *error_message = "failed to open output path";
        }
        return false;
    }

    output << "P3\n" << image.width << " " << image.height << "\n" << image.max_value << "\n";
    for (const Pixel& pixel : image.pixels) {
        output << pixel.r << " " << pixel.g << " " << pixel.b << "\n";
    }
    return true;
}

Image applyBoxBlur(const Image& input) {
    if (input.width <= 2 || input.height <= 2) {
        return input;
    }

    Image result = input;

    for (int y = 1; y < input.height - 1; ++y) {
        for (int x = 1; x < input.width - 1; ++x) {
            int sum_r = 0;
            int sum_g = 0;
            int sum_b = 0;
            int sample_count = 0;

            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    const int sx = x + dx;
                    const int sy = y + dy;
                    const Pixel& sample =
                        input.pixels[static_cast<std::size_t>(sy) * static_cast<std::size_t>(input.width) +
                                     static_cast<std::size_t>(sx)];
                    sum_r += sample.r;
                    sum_g += sample.g;
                    sum_b += sample.b;
                    ++sample_count;
                }
            }

            Pixel& target =
                result.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(input.width) + static_cast<std::size_t>(x)];
            target.r = sum_r / sample_count;
            target.g = sum_g / sample_count;
            target.b = sum_b / sample_count;
        }
    }

    return result;
}

Image upscaleNearestNeighbor(const Image& input, const int scale_factor) {
    Image output {
        .width = input.width * scale_factor,
        .height = input.height * scale_factor,
        .max_value = input.max_value,
        .pixels =
            std::vector<Pixel>(static_cast<std::size_t>(input.width * scale_factor) * static_cast<std::size_t>(input.height * scale_factor)),
    };

    for (int y = 0; y < output.height; ++y) {
        for (int x = 0; x < output.width; ++x) {
            const int sx = x / scale_factor;
            const int sy = y / scale_factor;
            output.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(output.width) + static_cast<std::size_t>(x)] =
                input.pixels[static_cast<std::size_t>(sy) * static_cast<std::size_t>(input.width) + static_cast<std::size_t>(sx)];
        }
    }

    return output;
}

contracts::EnhancementResult makeFailure(
    const contracts::ErrorCode code,
    const std::string& stage,
    const std::string& message) {
    contracts::EnhancementResult result {};
    result.ok = false;
    result.error.code = code;
    result.error.stage = stage;
    result.error.message = message;
    return result;
}

}  // namespace

contracts::EnhancementResult CpuStubPipeline::run(const contracts::EnhancementRequest& request) {
    const auto start_time = std::chrono::steady_clock::now();

    std::string reason;
    if (!contracts::isValidRequest(request, &reason)) {
        return makeFailure(contracts::ErrorCode::kInvalidRequest, "validate", reason);
    }

    Image decoded {};
    std::string io_error;
    if (!parsePpm(request.input_path, &decoded, &io_error)) {
        return makeFailure(contracts::ErrorCode::kDecodeFailed, "decode", io_error);
    }

    Image processed = decoded;
    if (request.denoise_enabled) {
        processed = applyBoxBlur(processed);
    }
    processed = upscaleNearestNeighbor(processed, request.scale_factor);

    if (!writePpm(processed, request.output_path, &io_error)) {
        return makeFailure(contracts::ErrorCode::kEncodeFailed, "encode", io_error);
    }

    const auto stop_time = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

    contracts::EnhancementResult result {};
    result.ok = true;
    result.output_path = request.output_path;
    result.metrics.input_width = decoded.width;
    result.metrics.input_height = decoded.height;
    result.metrics.output_width = processed.width;
    result.metrics.output_height = processed.height;
    result.metrics.duration_ms = static_cast<std::uint64_t>(elapsed.count());
    result.error.code = contracts::ErrorCode::kNone;
    result.error.stage = "none";
    return result;
}

}  // namespace lumos::engine

