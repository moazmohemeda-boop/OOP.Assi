#include <iostream>
using namespace std;
#include "Image_Class.h"

int main() {
    string filename;
    cout << "Pls enter colored image name: ";
    cin >> filename;
    Image image(filename);

    int choice;
    cout<< "enter the choice";
    cin>> choice;
    if (choice == 1) {   // grayscale

        for (int i = 0; i < image.width; ++i) {
            for (int j = 0; j < image.height; ++j) {
                unsigned  int avg = 0; // Initialize average value

                for (int k = 0; k < 3; ++k) {
                    avg += image(i, j, k); // Accumulate pixel values
                }

                avg /= 3; // Calculate average
                for (int k = 0; k < 3; ++k) {
                    image(i, j, k) = avg; // Set pixel values to average
                }
            }
            cout << "Pls enter image name to store new image\n";
            cout << " specify extension .jpg, .bmp, .png, .tga: ";

            cin >> filename;
            image.saveImage(filename);
        }



    }
    else if (choice == 2) {   // flip
        string flip;
        cout << "Vertical or Horizontal flip? (v/h): ";
        cin >> flip;

        if (flip == "v")
        {
            for (int i = 0; i < image.width; i++)
            {
                for (int j = 0; j < image.height / 2; j++)
                {
                    for (int k = 0; k < image.channels; k++) {
                        unsigned int temp = image(i, j, k);
                        image(i, j, k) = image(i, image.height - 1 - j , k);
                        image(i, image.height - 1 - j, k) = temp;
                    }
                }
            }
        }
        else if (flip == "h")
        {
            for (int i = 0; i < image.width / 2; i++)
            {
                for (int j = 0; j < image.height; j++)
                {
                    for (int k = 0; k < image.channels; k++)
                    {
                        unsigned int temp = image(i, j, k);
                        image(i, j, k) = image(image.width - 1 - i, j, k);
                        image(image.width - 1 - i, j, k) = temp;
                    }
                }

            }

        }
        string saved_image = "flipped_" + filename;
        image.saveImage(saved_image);
        cout << "Image saved as: " << saved_image << endl;
    }
}