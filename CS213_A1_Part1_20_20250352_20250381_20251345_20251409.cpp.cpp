/*
 * Team Members & IDs:
 * 1. Abdelrahman  - 20250352: Filters 1, 5, 9, 13
 * 2. Abdullah     - 20250381: Filters 2, 6, 10, 14, GUI
 * 3. Mohamed      - 20251345: Filters 3, 7, 11, 15, Filter 17, Filter 18
 * 4. Moaz Mohamed - 20251409: Filters 4, 8, 12, 16, Menu, GUI
 */

#define _USE_MATH_DEFINES
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include "Image_Class.h"

using namespace std;

void saveProcessedImage(Image& img) {
    string outName;
    cout << "Enter output image name (with extension .jpg, .bmp, .png, .tga): ";
    cin >> outName;
    img.saveImage(outName);
    cout << "Image saved successfully as: " << outName << "\n\n";
}

// Filter 1: Grayscale Conversion
Image grayscaleImage(const Image& img) {
    Image res(img.width, img.height);
    for (int x = 0; x < img.width; ++x) {
        for (int y = 0; y < img.height; ++y) {
            unsigned int avg = (img(x, y, 0) + img(x, y, 1) + img(x, y, 2)) / 3;
            res(x, y, 0) = avg;
            res(x, y, 1) = avg;
            res(x, y, 2) = avg;
        }
    }
    return res;
}

// Filter 2: Black and White
Image blackAndWhiteImage(const Image& img) {
    Image res(img.width, img.height);
    for (int x = 0; x < img.width; ++x) {
        for (int y = 0; y < img.height; ++y) {
            unsigned int avg = (img(x, y, 0) + img(x, y, 1) + img(x, y, 2)) / 3;
            unsigned int val = (avg > 127) ? 255 : 0;
            res(x, y, 0) = val;
            res(x, y, 1) = val;
            res(x, y, 2) = val;
        }
    }
    return res;
}

// Filter 3: Invert Image
Image invertImage(const Image& img) {
    Image res(img.width, img.height);
    for (int x = 0; x < img.width; ++x) {
        for (int y = 0; y < img.height; ++y) {
            for (int k = 0; k < 3; ++k) {
                res(x, y, k) = 255 - img(x, y, k);
            }
        }
    }
    return res;
}

// Filter 4: Adding a Frame
Image addFrameImage(const Image& img, int frameWidth = 15) {
    Image res = img;
    for (int x = 0; x < res.width; ++x) {
        for (int y = 0; y < res.height; ++y) {
            if (x < frameWidth || x >= res.width - frameWidth || y < frameWidth || y >= res.height - frameWidth) {
                res(x, y, 0) = 0;   
                res(x, y, 1) = 102; 
                res(x, y, 2) = 204; 
            }
        }
    }
    return res;
}

// Filter 5: Flip Image
Image flipImage(const Image& img, int mode) {
    Image res(img.width, img.height);
    for (int x = 0; x < img.width; ++x) {
        for (int y = 0; y < img.height; ++y) {
            for (int k = 0; k < 3; ++k) {
                if (mode == 1) {
                    res(img.width - 1 - x, y, k) = img(x, y, k);
                } else {
                    res(x, img.height - 1 - y, k) = img(x, y, k);
                }
            }
        }
    }
    return res;
}

// Filter 6: Rotate Image
Image rotateImage(const Image& img, int degree) {
    if (degree == 90) {
        Image res(img.height, img.width);
        for (int i = 0; i < img.width; i++) {
            for (int j = 0; j < img.height; j++) {
                for (int k = 0; k < 3; k++) {
                    res(res.width - 1 - j, res.height - 1 - i, k) = img(i, j, k);
                }
            }
        }
        return res;
    } else if (degree == 180) {
        Image res(img.width, img.height);
        for (int i = 0; i < img.width; i++) {
            for (int j = 0; j < img.height; j++) {
                for (int k = 0; k < 3; k++) {
                    res(res.width - 1 - i, res.height - 1 - j, k) = img(i, j, k);
                }
            }
        }
        return res;
    } else if (degree == 270) {
        Image res(img.height, img.width);
        for (int i = 0; i < img.width; i++) {
            for (int j = 0; j < img.height; j++) {
                for (int k = 0; k < 3; k++) {
                    res(j, res.height - 1 - i, k) = img(i, j, k);
                }
            }
        }
        return res;
    }
    return img;
}

// Filter 7: Darken and Lighten
Image adjustBrightness(const Image& img, bool lighten) {
    Image res(img.width, img.height);
    float factor = lighten ? 1.5f : 0.5f;
    for (int x = 0; x < img.width; ++x) {
        for (int y = 0; y < img.height; ++y) {
            for (int k = 0; k < 3; ++k) {
                int val = (int)(img(x, y, k) * factor);
                res(x, y, k) = min(255, max(0, val));
            }
        }
    }
    return res;
}

// Filter 8: Resizing Image
Image resizeImage(const Image& input, int newWidth, int newHeight) {
    Image output(newWidth, newHeight);
    float x_ratio = (float)input.width / newWidth;
    float y_ratio = (float)input.height / newHeight;
    for (int i = 0; i < newWidth; ++i) {
        for (int j = 0; j < newHeight; ++j) {
            float origX = i * x_ratio;
            float origY = j * y_ratio;
            for (int k = 0; k < input.channels; ++k) {
                output(i, j, k) = input((int)origX, (int)origY, k);
            }
        }
    }
    return output;
}

// Filter 9: Merge Images
Image mergeImages(Image img1, Image img2, int mode) {
    int width, height;
    if (mode == 1) {
        if (img1.width * img1.height < img2.width * img2.height) {
            img1 = resizeImage(img1, img2.width, img2.height);
        } else {
            img2 = resizeImage(img2, img1.width, img1.height);
        }
        width = img1.width;
        height = img1.height;
    } else {
        width = min(img1.width, img2.width);
        height = min(img1.height, img2.height);
    }
    Image merged(width, height);
    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            for (int k = 0; k < 3; ++k) {
                merged(i, j, k) = (img1(i, j, k) + img2(i, j, k)) / 2;
            }
        }
    }
    return merged;
}

// Filter 10: Detect Image Edges
Image detectEdges(const Image& img) {
    Image bw = blackAndWhiteImage(img);
    Image res(img.width, img.height);
    for (int i = 0; i < img.width - 1; ++i) {
        for (int j = 0; j < img.height - 1; ++j) {
            if (bw(i, j, 0) != bw(i + 1, j, 0) || bw(i, j, 0) != bw(i, j + 1, 0)) {
                res(i, j, 0) = 0;
                res(i, j, 1) = 0;
                res(i, j, 2) = 0;
            } else {
                res(i, j, 0) = 255;
                res(i, j, 1) = 255;
                res(i, j, 2) = 255;
            }
        }
    }
    return res;
}

// Filter 11: Crop Image
Image cropImage(const Image& img, int x, int y, int w, int h) {
    Image cropimg(w, h);
    for (int newx = 0; newx < w; ++newx) {
        for (int newy = 0; newy < h; ++newy) {
            for (int k = 0; k < img.channels; ++k) {
                cropimg(newx, newy, k) = img(x + newx, y + newy, k);
            }
        }
    }
    return cropimg;
}

// Filter 12: Blur Image (Strong Blur)
Image blurImage(const Image& img) {
    Image blurred(img.width, img.height);
    int radius = 6; // زيادة النطاق لضمان وضوح التأثير على الصور عالية الدقة

    for (int x = 0; x < img.width; ++x) {
        for (int y = 0; y < img.height; ++y) {
            for (int k = 0; k < 3; ++k) {
                int sum = 0;
                int count = 0;
                for (int dx = -radius; dx <= radius; ++dx) {
                    for (int dy = -radius; dy <= radius; ++dy) {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx >= 0 && nx < img.width && ny >= 0 && ny < img.height) {
                            sum += img(nx, ny, k);
                            count++;
                        }
                    }
                }
                blurred(x, y, k) = sum / count;
            }
        }
    }
    return blurred;
}

// Filter 13: Wano Sunlight Fix
Image sunlightImage(const Image& img) {
    Image res = img;
    for (int x = 0; x < res.width; x++) {
        for (int y = 0; y < res.height; y++) {
            for (int c = 0; c < 3; c++) {
                res(x, y, c) = min(255, res(x, y, c) + 40);
            }
            res(x, y, 0) = min(255, res(x, y, 0) + 20);
            res(x, y, 1) = min(255, res(x, y, 1) + 15);
        }
    }
    return res;
}

// Filter 14: Old TV / Den Den Mushi Effect
Image oldTVImage(const Image& img) {
    Image res = img;
    for (int x = 0; x < res.width; x++) {
        for (int y = 0; y < res.height; y++) {
            if (y % 4 == 0) {
                res(x, y, 0) = (int)(res(x, y, 0) * 0.6);
                res(x, y, 1) = (int)(res(x, y, 1) * 0.6);
                res(x, y, 2) = (int)(res(x, y, 2) * 0.6);
            }
        }
    }
    return res;
}

// Filter 15: Purple Tint Effect
Image purpleTintImage(const Image& img) {
    Image res = img;
    for (int i = 0; i < res.width; ++i) {
        for (int j = 0; j < res.height; ++j) {
            res(i, j, 0) = min(255, (int)(res(i, j, 0) * 1.6));
            res(i, j, 1) = (int)(res(i, j, 1) * 0.6);
            res(i, j, 2) = min(255, (int)(res(i, j, 2) * 1.8));
        }
    }
    return res;
}

// Filter 16: Infrared Photography Effect
Image infraredImage(const Image& img) {
    Image infrared(img.width, img.height);
    for (int i = 0; i < img.width; ++i) {
        for (int j = 0; j < img.height; ++j) {
            infrared(i, j, 0) = 255;
            infrared(i, j, 1) = 255 - img(i, j, 1);
            infrared(i, j, 2) = 255 - img(i, j, 2);
        }
    }
    return infrared;
}

// Filter 17: Oil Painting Effect
Image oilPaintImage(const Image& img) {
    int radius = 3, levels = 20;
    Image res(img.width, img.height);
    for (int x = 0; x < img.width; x++) {
        for (int y = 0; y < img.height; y++) {
            int cnt[256] = {0}, sum[256][3] = {0};
            for (int dx = -radius; dx <= radius; dx++) {
                for (int dy = -radius; dy <= radius; dy++) {
                    int nx = x + dx, ny = y + dy;
                    if (nx < 0 || ny < 0 || nx >= img.width || ny >= img.height) continue;
                    int r = img(nx, ny, 0), g = img(nx, ny, 1), b = img(nx, ny, 2);
                    int l = (r + g + b) / 3 * levels / 255;
                    cnt[l]++; 
                    sum[l][0] += r; 
                    sum[l][1] += g; 
                    sum[l][2] += b;
                }
            }
            int best = 0;
            for (int i = 1; i <= levels; i++) {
                if (cnt[i] > cnt[best]) best = i;
            }
            for (int c = 0; c < 3; c++) {
                res(x, y, c) = (cnt[best] > 0) ? (sum[best][c] / cnt[best]) : 0;
            }
        }
    }
    return res;
}

// Filter 18: Skew Image
Image skewImage(const Image& img, double angle) {
    double anglerad = angle * M_PI / 180.0;
    double angletan = tan(anglerad);
    int maxShift = (int)(angletan * img.height);
    if (maxShift < 0) maxShift = -maxShift;
    int newwidth = img.width + maxShift;
    Image skewedimage(newwidth, img.height);

    for (int i = 0; i < newwidth; ++i) {
        for (int j = 0; j < img.height; ++j) {
            for (int k = 0; k < img.channels; ++k) {
                skewedimage(i, j, k) = 255;
            }
        }
    }

    for (int y = 0; y < img.height; ++y) {
        int shift = (int)(y * angletan);
        if (shift < 0) shift = -shift;
        for (int x = 0; x < img.width; ++x) {
            int newx = x + shift;
            if (newx >= 0 && newx < newwidth) {
                for (int k = 0; k < img.channels; ++k) {
                    skewedimage(newx, y, k) = img(x, y, k);
                }
            }
        }
    }
    return skewedimage;
}

// Main Menu
int main() {
    while (true) {
        cout << "Menu\n";
        cout << "1. Grayscale Conversion\n";
        cout << "2. Black and White\n";
        cout << "3. Invert Image\n";
        cout << "4. Adding a Frame\n";
        cout << "5. Flip Image\n";
        cout << "6. Rotate Image\n";
        cout << "7. Darken and Lighten\n";
        cout << "8. Resizing Image\n";
        cout << "9. Merge Images\n";
        cout << "10. Detect Image Edges\n";
        cout << "11. Crop Image\n";
        cout << "12. Blur Image\n";
        cout << "13. Wano Sunlight Fix\n";
        cout << "14. Old TV / Den Den Mushi Effect\n";
        cout << "15. Purple Tint Effect\n";
        cout << "16. Infrared Photography Effect\n";
        cout << "17. Oil Painting Effect\n";
        cout << "18. Skew Image\n";
        cout << "0. Exit\n";
        cout << "Enter your choice (0-18): ";

        int choice;
        cin >> choice;

        if (choice == 0) {
            cout << "Exiting program. Goodbye!\n";
            break;
        }

        if (choice < 1 || choice > 18) {
            cout << "Invalid choice! Please try again.\n\n";
            continue;
        }

        string filename;
        cout << "Enter input image filename: ";
        cin >> filename;
        Image img(filename);

        switch (choice) {
            case 1: {
                Image res = grayscaleImage(img);
                saveProcessedImage(res);
                break;
            }
            case 2: {
                Image res = blackAndWhiteImage(img);
                saveProcessedImage(res);
                break;
            }
            case 3: {
                Image res = invertImage(img);
                saveProcessedImage(res);
                break;
            }
            case 4: {
                Image res = addFrameImage(img);
                saveProcessedImage(res);
                break;
            }
            case 5: {
                int mode;
                cout << "1) Horizontal Flip\n2) Vertical Flip\nChoice: ";
                cin >> mode;
                Image res = flipImage(img, mode);
                saveProcessedImage(res);
                break;
            }
            case 6: {
                int degree;
                cout << "Enter rotation degree (90, 180, 270): ";
                cin >> degree;
                Image res = rotateImage(img, degree);
                saveProcessedImage(res);
                break;
            }
            case 7: {
                int mode;
                cout << "1) Lighten\n2) Darken\nChoice: ";
                cin >> mode;
                Image res = adjustBrightness(img, mode == 1);
                saveProcessedImage(res);
                break;
            }
            case 8: {
                int nw, nh;
                cout << "Enter new width and height: ";
                cin >> nw >> nh;
                Image res = resizeImage(img, nw, nh);
                saveProcessedImage(res);
                break;
            }
            case 9: {
                string file2;
                cout << "Enter second image filename: ";
                cin >> file2;
                Image img2(file2);
                int mode;
                cout << "1) Resize smaller to match\n2) Crop common area\nChoice: ";
                cin >> mode;
                Image res = mergeImages(img, img2, mode);
                saveProcessedImage(res);
                break;
            }
            case 10: {
                Image res = detectEdges(img);
                saveProcessedImage(res);
                break;
            }
            case 11: {
                int x, y, w, h;
                cout << "Enter start X, start Y, Width, Height: ";
                cin >> x >> y >> w >> h;
                Image res = cropImage(img, x, y, w, h);
                saveProcessedImage(res);
                break;
            }
            case 12: {
                Image res = blurImage(img);
                saveProcessedImage(res);
                break;
            }
            case 13: {
                Image res = sunlightImage(img);
                saveProcessedImage(res);
                break;
            }
            case 14: {
                Image res = oldTVImage(img);
                saveProcessedImage(res);
                break;
            }
            case 15: {
                Image res = purpleTintImage(img);
                saveProcessedImage(res);
                break;
            }
            case 16: {
                Image res = infraredImage(img);
                saveProcessedImage(res);
                break;
            }
            case 17: {
                Image res = oilPaintImage(img);
                saveProcessedImage(res);
                break;
            }
            case 18: {
                double angle;
                cout << "Enter skew angle (in degrees): ";
                cin >> angle;
                Image res = skewImage(img, angle);
                saveProcessedImage(res);
                break;
            }
        }
    }
    return 0;
}