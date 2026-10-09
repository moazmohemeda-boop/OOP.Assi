/*
  Assignment 3 - Image Processing Program

  Team Members:
  1. Name: Moaz        | ID: 20251409 | Filters: 4, 8, 12, 16
  2. Name: Mohamed     | ID: 20251345 | Filters: 3, 7, 11, 15
  3. Name: Abdullah    | ID: 20250381 | Filters: 2, 6, 10, 14
  4. Name: Abdelrahman | ID: 20250352 | Filters: 1, 5, 9, 13
*/

#include <iostream>
#include <string>
#include "Image_Class.h"

using namespace std;

// Filter 1: Grayscale
void grayscaleImage(Image& img) {
    for (int i = 0; i < img.width; ++i) {
        for (int j = 0; j < img.height; ++j) {
            unsigned int avg = 0;
            for (int k = 0; k < 3; ++k) {
                avg += img(i, j, k);
            }
            avg /= 3;
            for (int k = 0; k < 3; ++k) {
                img(i, j, k) = avg;
            }
        }
    }
}

// Filter 2: Invert
void invertImage(Image& img) {
    for (int i = 0; i < img.width; ++i) {
        for (int j = 0; j < img.height; ++j) {
            for (int k = 0; k < 3; ++k) {
                img(i, j, k) = 255 - img(i, j, k);
            }
        }
    }
}

// Filter 3: Black and White
void blackAndWhiteImage(Image& img) {
    for (int i = 0; i < img.width; ++i) {
        for (int j = 0; j < img.height; ++j) {
            unsigned int avg = 0;
            for (int k = 0; k < 3; ++k) {
                avg += img(i, j, k);
            }
            avg /= 3;
            unsigned int val = (avg > 127) ? 255 : 0;
            img(i, j, 0) = val;
            img(i, j, 1) = val;
            img(i, j, 2) = val;
        }
    }
}

// Filter 4: Frame
void myFrame(Image& img) {
    int thickness = 20;
    for (int x = 0; x < img.width; ++x) {
        for (int y = 0; y < img.height; ++y) {
            if (x < thickness || x >= (img.width - thickness) || 
                y < thickness || y >= (img.height - thickness)) {
                img(x, y, 0) = 0;
                img(x, y, 1) = 0;
                img(x, y, 2) = 0;
            }
        }
    }
}

// Filter 5: Flip
void flipImage(Image& img, string flip) {
    if (flip == "v" || flip == "V") {
        for (int i = 0; i < img.width; i++) {
            for (int j = 0; j < img.height / 2; j++) {
                for (int k = 0; k < img.channels; k++) {
                    unsigned int temp = img(i, j, k);
                    img(i, j, k) = img(i, img.height - 1 - j, k);
                    img(i, img.height - 1 - j, k) = temp;
                }
            }
        }
    }
    else if (flip == "h" || flip == "H") {
        for (int i = 0; i < img.width / 2; i++) {
            for (int j = 0; j < img.height; j++) {
                for (int k = 0; k < img.channels; k++) {
                    unsigned int temp = img(i, j, k);
                    img(i, j, k) = img(img.width - 1 - i, j, k);
                    img(img.width - 1 - i, j, k) = temp;
                }
            }
        }
    }
}

// Filter 6: Rotate
Image rotateImage(Image& img, int degree) {
    if (degree == 90) {
        Image img90(img.height, img.width);
        for (int i = 0; i < img.width; i++) {
            for (int j = 0; j < img.height; j++) {
                for (int k = 0; k < img.channels; k++) {
                    img90(img90.width - 1 - j, img90.height - 1 - i, k) = img(i, j, k);
                }
            }
        }
        return img90;
    }
    else if (degree == 180) {
        Image img180(img.width, img.height);
        for (int i = 0; i < img.width; i++) {
            for (int j = 0; j < img.height; j++) {
                for (int k = 0; k < 3; k++) {
                    img180(i, img180.height - 1 - j, k) = img(i, j, k);
                }
            }
        }
        return img180;
    }
    else if (degree == 270) {
        Image img270(img.height, img.width);
        for (int i = 0; i < img.width; i++) {
            for (int j = 0; j < img.height; j++) {
                for (int k = 0; k < 3; k++) {
                    img270(j, img270.height - 1 - i, k) = img(i, j, k);
                }
            }
        }
        return img270;
    }
    return img;
}

// Filter 7: Lighten / Darken
void lightenDarkenImage(Image& img, int mode, double per) {
    if (mode == 1) {
        double factor = 1.0 + (per / 100.0);
        for (int i = 0; i < img.width; ++i) {
            for (int j = 0; j < img.height; ++j) {
                for (int k = 0; k < 3; ++k) {
                    int newvalue = img(i, j, k) * factor;
                    if (newvalue > 255) newvalue = 255;
                    img(i, j, k) = newvalue;
                }
            }
        }
    }
    else if (mode == 2) {
        double factor = 1.0 - (per / 100.0);
        for (int i = 0; i < img.width; ++i) {
            for (int j = 0; j < img.height; ++j) {
                for (int k = 0; k < 3; ++k) {
                    int newvalue = img(i, j, k) * factor;
                    if (newvalue < 0) newvalue = 0;
                    img(i, j, k) = newvalue;
                }
            }
        }
    }
}

// Filter 8: Resize
Image resizeImage(Image& oldPic, int w, int h) {
    Image newPic(w, h);
    float rx = (float)oldPic.width / w;
    float ry = (float)oldPic.height / h;

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            int ox = x * rx;
            int oy = y * ry;

            newPic(x, y, 0) = oldPic(ox, oy, 0);
            newPic(x, y, 1) = oldPic(ox, oy, 1);
            newPic(x, y, 2) = oldPic(ox, oy, 2);
        }
    }

    return newPic;
}

// Menu
int main() {
    int choice = 0;

    while (true) {
        cout << "\nSelect Filter:\n";
        cout << "1. Grayscale Image\n";
        cout << "2. Invert Image\n";
        cout << "3. Black and White Image\n";
        cout << "4. Add Frame\n";
        cout << "5. Flip Image\n";
        cout << "6. Rotate Image\n";
        cout << "7. Lighten / Darken Image\n";
        cout << "8. Resize Image\n";
        cout << "0. Exit\n";
        cout << "Choice: ";
        cin >> choice;

        if (choice == 0) {
            cout << "Exiting...\n";
            break;
        }

        if (choice < 1 || choice > 8) {
            cout << "Invalid choice, try again.\n";
            continue;
        }

        string imageName;
        cout << "Enter image name: ";
        cin >> imageName;

        Image img(imageName);
        string saveName;

        switch (choice) {
            case 1:
                grayscaleImage(img);
                cout << "Enter output image name: ";
                cin >> saveName;
                img.saveImage(saveName);
                break;
            case 2:
                invertImage(img);
                cout << "Enter output image name: ";
                cin >> saveName;
                img.saveImage(saveName);
                break;
            case 3:
                blackAndWhiteImage(img);
                cout << "Enter output image name: ";
                cin >> saveName;
                img.saveImage(saveName);
                break;
            case 4:
                myFrame(img);
                cout << "Enter output image name: ";
                cin >> saveName;
                img.saveImage(saveName);
                break;
            case 5: {
                string flipDir;
                cout << "Flip Vertical or Horizontal? (v/h): ";
                cin >> flipDir;
                flipImage(img, flipDir);
                cout << "Enter output image name: ";
                cin >> saveName;
                img.saveImage(saveName);
                break;
            }
            case 6: {
                int degree;
                cout << "Enter degree (90, 180, 270): ";
                cin >> degree;
                Image rotated = rotateImage(img, degree);
                cout << "Enter output image name: ";
                cin >> saveName;
                rotated.saveImage(saveName);
                break;
            }
            case 7: {
                int mode;
                double per;
                cout << "1 for Lighten, 2 for Darken: ";
                cin >> mode;
                cout << "Enter percentage: ";
                cin >> per;
                lightenDarkenImage(img, mode, per);
                cout << "Enter output image name: ";
                cin >> saveName;
                img.saveImage(saveName);
                break;
            }
            case 8: {
                int w, h;
                cout << "Enter width and height: ";
                cin >> w >> h;
                Image resized = resizeImage(img, w, h);
                cout << "Enter output image name: ";
                cin >> saveName;
                resized.saveImage(saveName);
                break;
            }
        }
    }

    return 0;
}