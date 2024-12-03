#include <opencv2/opencv.hpp>
#include<iostream>

using namespace std;
using namespace cv;

class SeamCarver{
    private:

    public :
    Mat image;
    double** dp_vertical = NULL;
    double** dp_horizontal = NULL;
    double** energyMatrix = NULL;
    
    SeamCarver(string img_location){
        this->image = imread(img_location);
        if(image.empty()){
            cout<<"Some issue with image path or permission"<<endl;
            exit(1);
        }
    }

    //Free existing space in heap i.e. removing memory leak
    // void freeEnergyMatrixHeap(){
    //     int row_length = image.rows;
    //     int col_length = image.cols;

    //     if(energyMatrix != NULL){
    //         for(int i=0; i<row_length; i++){
    //             delete(energyMatrix[i]);
    //         }
    //         delete(energyMatrix);
    //     }

    // }

    //Free existing space in heap i.e. removing memory leak
    // void freeDPMatrixHeap(){
    //     int row_length = image.rows;
    //     int col_length = image.cols;

    //     if(dp != NULL){
    //         for(int i=0; i<col_length; i++){
    //             delete(dp[i]);
    //         }
    //         delete(dp);
    //     }
    // }

    //Get Mat object for image
    Mat getPicture(){
        return image;
    }

    //Get number of columns of pixels in the image
    int width(){
        return image.cols;
    }

    //Get number of rows of pixels in the image
    int height(){
        return image.rows;
    }

    //Get energy of an index (x,y) of the image matrix
    double energy(int x, int y){
        int row_length = image.rows;
        int col_length = image.cols;

        Vec3b left_pixel = image.at<Vec3b>(x, (y+col_length-1)%col_length);
        Vec3b right_pixel = image.at<Vec3b>(x, (y+1)%col_length);
        Vec3b above_pixel = image.at<Vec3b>((x+row_length-1)%row_length, y);
        Vec3b below_pixel = image.at<Vec3b>((x+1)%row_length, y);

        int delta_blue_x = abs(right_pixel[0] - left_pixel[0]);
        int delta_green_x = abs(right_pixel[1] - left_pixel[1]);
        int delta_red_x = abs(right_pixel[2] - left_pixel[2]);
        int delta_x_square = pow(delta_blue_x, 2) + pow(delta_green_x, 2) + pow(delta_red_x, 2);

        int delta_blue_y = abs(above_pixel[0] - below_pixel[0]);
        int delta_green_y = abs(above_pixel[1] - below_pixel[1]);
        int delta_red_y = abs(above_pixel[2] - below_pixel[2]);
        int delta_y_square = pow(delta_blue_y, 2) + pow(delta_green_y, 2) + pow(delta_red_y, 2);

        return sqrt(delta_x_square + delta_y_square);
    }

    //Create Energy Matrix for image
    void createEnergyMatrix(){
        int row_length = image.rows;
        int col_length = image.cols;
        energyMatrix = new double*[row_length];

        //Initialize pointer to pointer energy matrix
        for (int i = 0; i < row_length; i++) {
            energyMatrix[i] = new double[col_length];
        }

        for (int i=0; i<row_length; i++) {
            for (int j=0; j<col_length; j++){
                energyMatrix[i][j] = energy(i, j);
            }      
        }
    }

    //Create DP Matrix for vertical seam
    void createDP_vertical(){
        int row_length = image.rows;
        int col_length = image.cols;
        dp_vertical = new double*[row_length];
        
        for (int i=0; i<row_length; i++) {
            dp_vertical[i] = new double[col_length];
        }

        for (int i=0; i<row_length; i++) {
            for (int j=0; j<col_length; j++){
                if(i!=0){
                    double val1 = j-1>=0 ? dp_vertical[i-1][j-1] : DBL_MAX; 
                    double val2 = dp_vertical[i-1][j];
                    double val3 = j+1<col_length ? dp_vertical[i-1][j+1] : DBL_MAX;

                    dp_vertical[i][j] = energyMatrix[i][j] + min(val1, min(val2, val3));
                }else{
                    dp_vertical[i][j] = energyMatrix[i][j];
                }
            }
        }
    }

    //Create DP Matrix for horizontal seam
    void createDP_horizontal(){
        int row_length = image.rows;
        int col_length = image.cols;
        dp_horizontal = new double*[row_length];
        
        for (int i=0; i<row_length; i++) {
            dp_horizontal[i] = new double[col_length];
        }

        for (int j=0; j<col_length; j++) {
            for (int i=0; i<row_length; i++){
                if(j!=0){
                    double val1 = i-1>=0 ? dp_horizontal[i-1][j-1] : DBL_MAX; 
                    double val2 = dp_horizontal[i][j-1];
                    double val3 = i+1<row_length ? dp_horizontal[i+1][j-1] : DBL_MAX;

                    dp_horizontal[i][j] = energyMatrix[i][j] + min(val1, min(val2, val3));
                }else{
                    dp_horizontal[i][j] = energyMatrix[i][j];
                }
            }
        }
    }

    //Find vertical seam and making pixels red to show which seam is selected
    int* findVerticalSeam(){
        int row_length = image.rows;
        int col_length = image.cols;
        int* verticalSeam = new int[row_length];

        int minVal = dp_vertical[row_length-1][0];
        for(int j=1;j<col_length;j++){
            if(dp_vertical[row_length-1][j] < minVal){
                minVal = dp_vertical[row_length-1][j];
                verticalSeam[row_length-1] = j;
            }
        }

        //Index to store in vertical seam array 
        int idx=row_length-2;

        //i and j indices for starting point in dp matrix
        int i=row_length-1;
        int j=verticalSeam[row_length-1];
        
        while(i>0 && j>=0 && j<col_length){
            double val1 = j-1>=0 ? dp_vertical[i-1][j-1] : DBL_MAX; 
            double val2 = dp_vertical[i-1][j];
            double val3 = j+1<col_length ? dp_vertical[i-1][j+1] : DBL_MAX;

            if(val1<val2 && val1<val3){
                Vec3b & pixel = image.at<Vec3b>(i, j);
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 255;
                verticalSeam[idx--] = j-1;
                j=j-1;
            }else if(val2<val1 && val2<val3){
                Vec3b & pixel = image.at<Vec3b>(i, j);
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 255;
                verticalSeam[idx--] = j;
            }else{
                Vec3b & pixel = image.at<Vec3b>(i, j);
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 255;
                verticalSeam[idx--] = j+1;
                j=j+1;
            }
            i--;
        }

        return verticalSeam;
       
    }

    //Remove Vertical seam and then create new image
    void removeVerticalSeam(int* seam){
        int row_length = image.rows;
        int col_length = image.cols;

        //Remove Vertical seam
        Mat modifiedImage(row_length, col_length - 1, image.type());

        // Copy pixels from originalImage to modifiedImage, excluding the specified column
        for (int i=0; i<row_length; i++) {
            for (int j=0, k=0; j<col_length; j++) {
                if (j != seam[i]) {
                    modifiedImage.at<Vec3b>(i, k) = image.at<Vec3b>(i, j);
                    k++;
                }
            }
        }

        image = modifiedImage;
    }

    //Find horizontal seam
    int* findHorizontalSeam(){
        int row_length = image.rows;
        int col_length = image.cols;
        int* horizontalSeam = new int[col_length];

        int minVal = dp_horizontal[0][col_length-1];
        for(int i=1;i<row_length;i++){
            if(dp_horizontal[i][col_length-1] < minVal){
                minVal = dp_horizontal[i][col_length-1];
                horizontalSeam[col_length-1] = i;
            }
        }

        //Index to store in vertical seam array 
        int idx=col_length-2;

        //i and j indices for starting point in dp matrix
        int j=col_length-1;
        int i=horizontalSeam[col_length-1];

        while(j>0 && i>=0 && i<row_length){
            double val1 = i-1>=0 ? dp_horizontal[i-1][j-1] : DBL_MAX; 
            double val2 = dp_horizontal[i][j-1];
            double val3 = i+1<row_length-1 ? dp_horizontal[i+1][j-1] : DBL_MAX;

            if(val1<val2 && val1<val3){
                Vec3b & pixel = image.at<Vec3b>(i, j);
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 255;
                horizontalSeam[idx--] = i-1;
                i=i-1;
            }else if(val2<val1 && val2<val3){
                Vec3b & pixel = image.at<Vec3b>(i, j);
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 255;
                horizontalSeam[idx--] = i;
            }else{
                Vec3b & pixel = image.at<Vec3b>(i, j);
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 255;
                horizontalSeam[idx--] = i+1;
                i=i+1;
            }
            j--;
        }

        return horizontalSeam;
    }

    //Remove Horizontal seam and then create new image
    void removeHorizontalSeam(int* seam){
        int row_length = image.rows;
        int col_length = image.cols;

        //Remove Vertical seam
        Mat modifiedImage(row_length - 1, col_length, image.type());

        // Copy pixels from originalImage to modifiedImage, excluding the specified column
        for (int j=0; j<col_length; j++) {
            for (int i=0, k=0; i<row_length; i++) {
                if (i != seam[j]) {
                    modifiedImage.at<Vec3b>(k, j) = image.at<Vec3b>(i, j);
                    k++;
                }
            }
        }

        image = modifiedImage;
    }

};

int main(){
    int final_width;
    int final_height;
    string image;

    cout<<"Enter Pic path : ";
    cin>>image;

    cout<<"Enter width : "<<endl;
    cin>>final_width;

    cout<<"Enter height : "<<endl;
    cin>>final_height;

    SeamCarver sc(image);
    Mat img = sc.getPicture();
    int row_length = img.rows;
    int col_length = img.cols;


    for(int i=0;i<col_length-final_width;i++){
        img = sc.getPicture();
        sc.createEnergyMatrix();
        sc.createDP_vertical();
        int* seam = sc.findVerticalSeam();
        imshow("image", img);
        waitKey(1);
        sc.removeVerticalSeam(seam);
    }

    for(int i=0;i<row_length-final_height;i++){
        img = sc.getPicture();
        sc.createEnergyMatrix();
        sc.createDP_horizontal();
        int* seam = sc.findHorizontalSeam();
        imshow("image", img);
        waitKey(1);
        sc.removeHorizontalSeam(seam);
    }

    // for(int i=0;i<row_length;i++){
    //     for(int j=0;j<col_length;j++){
    //         cout<<sc.energyMatrix[i][j]<<"  ";
    //     }
    //     cout<<endl;
    // }

    // cout<<endl;
    // cout<<endl;

    // for(int i=0;i<row_length;i++){
    //     for(int j=0;j<col_length;j++){
    //         cout<<sc.dp_vertical[i][j]<<"  ";
    //     }
    //     cout<<endl;
    // }

    img = sc.getPicture();
    imshow("image", img);
    waitKey(1000);

    bool check = imwrite("output.jpg", img);
    if(check){
        cout<<"image saved"<<endl;
    }else{
        cout<<"error in saving"<<endl;
    }
    

    return 0;
}

