
#include "Image_Class.h"
#include <iostream>
using namespace std;

int main() {
    Image image("mario.bmp");

    int choice;
    cout << "1) Invert   2) Lighten/Darken\nChoose: ";
    cin >> choice;

    if (choice == 1) {
        for (int i = 0; i < image.width; ++i) {
            for (int j = 0; j < image.height; ++j) {
                for (int k = 0; k < 3; ++k) {
                    image(i, j, k) = 255 - image(i, j, k);
                }
            }
        }
        image.saveImage("mario_invert.bmp");
    }
    else if (choice == 2) {
        int lighten, newvalue;
        double per;

        cout << "  if you want lighten enter 1 or  if you want darken enter 2 "<<endl;
        cin >> lighten;

        if (lighten == 1) {
            cout << "enter the percentage please"<<endl;
            cin >> per;
            per = 1.0 + (per / 100.0);

            for (int i = 0; i < image.width; ++i) {
                for (int j = 0; j < image.height; ++j) {
                    for (int k = 0; k < 3; ++k) {
                        newvalue = image(i, j, k) * per;
                        if (newvalue > 255) { newvalue = 255; }
                        image(i, j, k) = newvalue;
                    }
                }
            }
        }
        else {
            cout << "enter the percentage pls"<<endl;
            cin >> per;
            per = 1.0 - (per / 100.0);

            for (int i = 0; i < image.width; ++i) {
                for (int j = 0; j < image.height; ++j) {
                    for (int k = 0; k < 3; ++k) {
                        newvalue = image(i, j, k) * per;
                        if (newvalue < 0) { newvalue = 0; }
                        image(i, j, k) = newvalue;
                    }
                }
            }
        }
        image.saveImage("black_mario.bmp");
    }

    return 0;
}
