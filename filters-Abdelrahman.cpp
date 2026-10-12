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
else if( choice == 3) {                  //Merge imgage

    auto resizeImage = [&](const Image& input, int newWidth, int newHeight) {
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
    };
    string file1, file2;
    cout << "Enter first image: ";
    cin >> file1;
    cout << "Enter second image: ";
    cin >> file2;

    Image image1(file1);
    Image image2(file2);

    cout << "1)resize smaller\n2)crop bigger\n";
    int choice;
    cin >> choice;
    int width, height;
    if (choice == 1) {
        if (image1.width * image1.height < image2.width * image2.height) {
            image1 = resizeImage(image1, image2.width, image2.height);
        } else {
            image2= resizeImage(image2, image1.width, image1.height);
        }
        width = image1.width;
        height = image1.height;
    }
    else if (choice == 2) {
        width = min(image1.width, image2.width);
        height = min(image1.height, image2.height);
    }
    else {
        return 0;
    }
    Image merged(width, height);

    for (int i = 0; i < width; ++i) {
        for (int j = 0; j < height; ++j) {
            for (int k = 0; k < 3; ++k) {
                merged(i, j, k) = (image1(i, j, k) + image2(i, j, k)) / 2;
            }
        }
    }
    string outFile;
    cout << "Enter the name of the new image (include extension .jpg, .bmp, .png, .tga): ";
    cin >> outFile;
    merged.saveImage(outFile);
}
else if( choice == 4) {                 //Sunlight

    for (int x = 0; x < image.width; x++) {
        for (int y = 0; y < image.height; y++) {
            for (int c = 0; c < 3; c++) {
                image(x, y, c) = min(255, image(x, y, c) + 40);
            }
            image(x, y, 0) = min(255, image(x, y, 0) + 20);
            image(x, y, 1) = min(255, image(x, y, 1) + 15);
        }
    }
    string outFile;
    cout << "Enter the name of the new image (include extension .jpg, .bmp, .png, .tga): ";
    cin >> outFile;
    image.saveImage(outFile);

}
}


