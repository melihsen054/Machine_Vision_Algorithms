#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace cv;
using namespace std;

Mat test_images[10000];
Mat gaussnoised_timg[10000];
Mat spnoised_timg[10000];
Mat gausslinearfiltered_timg[10000];
Mat splinearfiltered_timg[10000];
Mat gaussnonlinearfiltered_timg[10000];
Mat spnonlinearfiltered_timg[10000];
Mat spnonlinear_linearfiltered_timg[10000];
Mat gausslinear_nonlinearfiltered_timg[10000];
char test_labels[10000];
uint32_t magic1;
uint32_t num_items1;
uint32_t rows1;
uint32_t cols1;
void read_data(const char* image_filename, const char* label_filename){
	auto reverseInt = [](int i) {
        unsigned char c1, c2, c3, c4;
        c1 = i & 255, c2 = (i >> 8) & 255, c3 = (i >> 16) & 255, c4 = (i >> 24) & 255;
        return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
    };
    // Open files
    std::ifstream image_file(image_filename,  std::ios::binary);
    std::ifstream label_file(label_filename,  std::ios::binary);

    // Read the magic and the meta data
    uint32_t magic;
    uint32_t num_items;
    uint32_t num_labels;
    uint32_t rows;
    uint32_t cols;

    image_file.read((char*)&magic, sizeof(magic));
	magic1=magic;
    magic = reverseInt(magic);
    if(magic != 2051){
        cout<<"Incorrect image file magic: "<<magic<<endl;
        return;
    }

    label_file.read((char*)&magic, sizeof(magic));
    magic = reverseInt(magic);
    if(magic != 2049){
        cout<<"Incorrect label file magic: "<<magic<<endl;
        return;
    }

    image_file.read((char*)&num_items, sizeof(num_items));
	num_items1=num_items;
    num_items = reverseInt(num_items);
    label_file.read((char*)(&num_labels),sizeof(num_labels));
    num_labels = reverseInt(num_labels);
    if(num_items != num_labels){
        cout<<"image file nums should equal to label num"<<endl;
        return;
    }

    image_file.read((char*)(&rows), 4);
	rows1=rows;
    rows = reverseInt(rows);
    image_file.read((char*)(&cols), 4);
	cols1=cols;
    cols = reverseInt(cols);

    char label;
    char* pixels = new char[rows * cols];
    for (int item_id = 0; item_id < num_items; ++item_id) {
        image_file.read(pixels, rows * cols);
        label_file.read(&label, 1);
		test_labels[item_id]=label;
        Mat image_tmp(rows,cols,CV_8UC1,pixels);
		test_images[item_id]=image_tmp.clone();
    }
    delete[] pixels;
}
void linearFilter(int mode){
	Mat matrix= Mat(3,3,CV_32F,(float)1/9);
		for(int a=0;a<10000;a++){
			if(mode==1)
				gausslinearfiltered_timg[a]=gaussnoised_timg[a].clone();
			else if(mode==2)
				splinearfiltered_timg[a]=spnoised_timg[a].clone();
			else if(mode==3)
				spnonlinear_linearfiltered_timg[a]=gaussnoised_timg[a].clone();
			for(int i=0;i<test_images[a].rows;i++){
				for(int k=0;k<test_images[a].cols;k++){
					float sum=0;
					float div=0;			
					for(int m=-1;m<2;m++){
						for(int n=-1;n<2;n++){
							//Controlling a valid pixel or not
							if((i+m)>=0&&(i+m)<test_images[a].rows&&(k+n)>=0&&(k+n)<test_images[a].cols){								
								if(mode==1)								
									sum+= (gausslinearfiltered_timg[a].at<uchar>(i+m,k+n)*matrix.at<float>(m+1,n+1));
								else if(mode==2)
									sum+= (splinearfiltered_timg[a].at<uchar>(i+m,k+n)*matrix.at<float>(m+1,n+1));
								else if(mode==3)
									sum+= (spnonlinearfiltered_timg[a].at<uchar>(i+m,k+n)*matrix.at<float>(m+1,n+1));
								div+=((float)1/9);
							}
						}	
					}					
					if(mode==1)
						gausslinearfiltered_timg[a].at<uchar>(i,k)=(int)(sum/div);
					else if(mode==2)
						splinearfiltered_timg[a].at<uchar>(i,k)=(int)(sum/div);
					else if(mode==3)
						spnonlinear_linearfiltered_timg[a].at<uchar>(i,k)=(int)(sum/div);
				}
			}		
		}	
}
int sortandselectmedian(int pixel[9], int numofpixel){
	for(int i=0;i<numofpixel;i++){
		int min=255;
		int index=i;
		for(int k=i;k<numofpixel;k++){
			if(pixel[k]<min){
				min=pixel[k];
				index=k;
			}
		}
		int temp=pixel[i];
		pixel[i]=min;
		pixel[index]=temp;
	}
	int median;
	if(numofpixel%2==0){
		median= (pixel[numofpixel/2]+pixel[(numofpixel/2)-1])/2;
	}else{
		median= pixel[int(numofpixel/2)];
	}
	return median;
}
void nonlinearFilter(int mode){//Median Filter
	for(int a=0;a<10000;a++){
		if(mode==1)
			gaussnonlinearfiltered_timg[a]=gaussnoised_timg[a].clone();
		else if(mode==2)
			spnonlinearfiltered_timg[a]=spnoised_timg[a].clone();
		else if(mode==3)
			gausslinear_nonlinearfiltered_timg[a]=gausslinearfiltered_timg[a].clone();
		for(int i=0;i<test_images[a].rows;i++){
			for(int k=0;k<test_images[a].cols;k++){
				int pixel[9];
				int numofpixel=0;			
				for(int m=-1;m<2;m++){
					for(int n=-1;n<2;n++){
						//Controlling a valid pixel or not
						if((i+m)>=0&&(i+m)<test_images[a].rows&&(k+n)>=0&&(k+n)<test_images[a].cols){
							if(mode==1)
								pixel[numofpixel]=gaussnoised_timg[a].at<uchar>(i+m,k+n);
							else if(mode==2)
								pixel[numofpixel]=spnoised_timg[a].at<uchar>(i+m,k+n);
							else if(mode==3)
								pixel[numofpixel]=gausslinearfiltered_timg[a].at<uchar> (i+m,k+n);														
							numofpixel++;							
						}
					}	
				}
				if(mode==1)
					gaussnonlinearfiltered_timg[a].at<uchar>(i,k)=sortandselectmedian(pixel,numofpixel);
				else if(mode==2)					
					spnonlinearfiltered_timg[a].at<uchar>(i,k)=sortandselectmedian(pixel,numofpixel);
				else if(mode==3)
					gausslinear_nonlinearfiltered_timg[a].at<uchar>(i,k)=sortandselectmedian(pixel,numofpixel);
			}
		}		
	}	
}
void savetodatabase(int mode){
	auto reverseInt = [](int i) {
        unsigned char c1, c2, c3, c4;
        c1 = i & 255, c2 = (i >> 8) & 255, c3 = (i >> 16) & 255, c4 = (i >> 24) & 255;
        return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
    };
	const char* filename ;
	if(mode==1)
		filename = "Database/MNIST/gaussnoisefiltered-images.idx3-ubyte";	
	else if(mode==2)
		filename = "Database/MNIST/spnoisefiltered-images.idx3-ubyte";
	
	// Open files
	FILE* FP = fopen(filename,"wb");
	
	
	uint32_t num_items =10000;
    uint32_t rows = 28;
    uint32_t cols = 28;
	
	
	uint32_t vararray[4]={magic1,num_items1,rows1,cols1};
	fwrite(vararray, 4, sizeof(uint32_t), FP);
  	for(int i=0;i<num_items;i++){
		if(mode==1)
		    fwrite(gausslinear_nonlinearfiltered_timg[i].data, cols * rows, 1, FP);
		else if(mode==2)
			fwrite(spnonlinear_linearfiltered_timg[i].data, cols * rows, 1, FP);
	}
	fclose(FP);

}
int main(){
	read_data("Database/MNIST/t10k-images.idx3-ubyte", "Database/MNIST/t10k-labels.idx1-ubyte");
	//Adding gaussian noise to test images	
	for(int i=0;i<10000;i++){
		cv::Mat noise(test_images[i].size(),test_images[i].type());
		cv::randn(noise,0,50);
		gaussnoised_timg[i] = test_images[i]+noise;
	}	
	//Adding saltpepper noise to test images	
	for(int i=0;i<10000;i++){
		Mat saltpepper_noise = Mat::zeros(test_images[i].size(),CV_8U);
		randu(saltpepper_noise,0,255);
		Mat black = saltpepper_noise < 20;
		Mat white = saltpepper_noise > 235;
		spnoised_timg[i] = test_images[i].clone();
		spnoised_timg[i].setTo(255,white);
		spnoised_timg[i].setTo(0,black);
	}
	linearFilter(1);//gaussian noise linear filtered
	linearFilter(2);//sp noise linear filtered
	nonlinearFilter(1);//gaussian noise nonlinear filtered
	nonlinearFilter(2);//sp noise nonlinear filtered
	nonlinearFilter(3);//gaussian linear filtered image filtered by nonlinear filter 
	linearFilter(3);//sp nonlinear filtered image filtered by linear filter
	Mat image11;
	Mat image12;
	Mat image13;
	Mat image14;
	Mat image15;
	Mat image16;
	Mat image17;
	Mat image18;
	Mat image19;
	int value = 540; // I just choose this number to give example
	cv::resize(test_images[value],image11 , cv::Size(150, 150));
	imshow ("Original image",image11);
	waitKey(0);
	cv::resize(gaussnoised_timg[value],image12 , cv::Size(150, 150));
	imshow ("Gaussian noise",image12);
	waitKey(0);
	cv::resize(gausslinearfiltered_timg[value],image13 , cv::Size(150, 150));	
	imshow ("Gaussian noise Linear Filtered",image13);
	waitKey(0);
	cv::resize(gaussnonlinearfiltered_timg[value],image14 , cv::Size(150, 150));	
	imshow ("Gaussian noise nonLinear Filtered",image14);
	waitKey(0);
	cv::resize(gausslinear_nonlinearfiltered_timg[value],image15 , cv::Size(150, 150));
	imshow ("Gauss Linear+Nonlinear",image15);
	waitKey(0);	
	cv::resize(spnoised_timg[value],image16 , cv::Size(150, 150));
	imshow ("S&P noise",image16);
	waitKey(0);
	cv::resize(splinearfiltered_timg[value],image17 , cv::Size(150, 150));	
	imshow ("S&P noise Linear Filtered",image17);
	waitKey(0);
	cv::resize(spnonlinearfiltered_timg[value],image18 , cv::Size(150, 150));	
	imshow ("S&P noise nonLinear Filtered",image18);
	waitKey(0);
	cv::resize(spnonlinear_linearfiltered_timg[value],image19 , cv::Size(150, 150));	
	imshow ("S&P linear+nonLinear",image19);
	waitKey(0);
	
	savetodatabase(1);//gauss noise filtered images
	savetodatabase(2);//sp noise filtered images
	int sum1=0;
	int sum2=0;
	int sum3=0;
	int sum4=0;	
	int sum5=0;
	int sum6=0;	
	for(int i=0;i<10000;i++){		
		for(int a=0;a<28;a++){
			for(int b=0;b<28;b++){
				sum1 += abs(gaussnoised_timg[i].at<uchar>(a,b)-test_images[i].at<uchar>(a,b));
				sum2 += abs(gausslinearfiltered_timg[i].at<uchar>(a,b)-test_images[i].at<uchar>(a,b));
				sum3 += abs(gaussnonlinearfiltered_timg[i].at<uchar>(a,b)-test_images[i].at<uchar>(a,b));
				sum4 += abs(spnoised_timg[i].at<uchar>(a,b)-test_images[i].at<uchar>(a,b));
				sum5 += abs(splinearfiltered_timg[i].at<uchar>(a,b)-test_images[i].at<uchar>(a,b));
				sum6 += abs(spnonlinearfiltered_timg[i].at<uchar>(a,b)-test_images[i].at<uchar>(a,b));
			}
		}
	}
	float differance1= (float)sum1/(float) (10000*28*28);
	float differance2= (float)sum2/(float) (10000*28*28);
	float differance3= (float)sum3/(float) (10000*28*28);
	float differance4= (float)sum4/(float) (10000*28*28);	
	float differance5= (float)sum5/(float) (10000*28*28);
	float differance6= (float)sum6/(float) (10000*28*28);	
	cout<<"Differance #1: "<< differance1<<endl;
	cout<<"Differance #2: "<< differance2<<endl;
	cout<<"Differance #3: "<< differance3<<endl;
	cout<<"Differance #4: "<< differance4<<endl; 		
	cout<<"Differance #5: "<< differance5<<endl;
	cout<<"Differance #6: "<< differance6<<endl;	
	return 0;
}


