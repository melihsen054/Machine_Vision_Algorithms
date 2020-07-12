#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

Mat image1;
int label=0;
int contours[100][1000][2];
int stack[1000][2], top=-1;
Mat test_images[10000];
char test_labels[10000];
void read_data(const char* image_filename, const char* label_filename){
	auto reverseInt = [](int i) {
        unsigned char c1, c2, c3, c4;
        c1 = i & 255, c2 = (i >> 8) & 255, c3 = (i >> 16) & 255, c4 = (i >> 24) & 255;
        return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
    };
    // Open files
    std::ifstream image_file(image_filename, std::ios::in | std::ios::binary);
    std::ifstream label_file(label_filename, std::ios::in | std::ios::binary);

    // Read the magic and the meta data
    uint32_t magic;
    uint32_t num_items;
    uint32_t num_labels;
    uint32_t rows;
    uint32_t cols;

    image_file.read((char*)&magic, sizeof(magic));
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
    num_items = reverseInt(num_items);
    label_file.read((char*)(&num_labels),sizeof(num_labels));
    num_labels = reverseInt(num_labels);
    if(num_items != num_labels){
        cout<<"image file nums should equal to label num"<<endl;
        return;
    }

    image_file.read(reinterpret_cast<char*>(&rows), 4);
    rows = reverseInt(rows);
    image_file.read(reinterpret_cast<char*>(&cols), 4);
    cols = reverseInt(cols);

    char label;
    char* pixels = new char[rows * cols];
    for (int item_id = 0; item_id < num_items; ++item_id) {
        // read image pixel
        image_file.read(pixels, rows * cols);
        // read label
        label_file.read(&label, 1);
		test_labels[item_id]=label;
        //string sLabel = std::to_string(int(label));
        //cout<<"lable is: "<<sLabel<<endl;
        // convert it to cv Mat, and show it
        Mat image_tmp(rows,cols,CV_8U,pixels);
		test_images[item_id]=image_tmp.clone();
        //cv::resize(image_tmp, image_tmp, cv::Size(150, 150));
        //cv::imshow(sLabel, image_tmp);
        //cv::waitKey(0);
    }
    delete[] pixels;
}
int push(int x,int y) {
   	top++;
   	stack[top][0]=x;
	stack[top][1]=y;
	return 0;
}
int popx() {
   	int value =stack[top][0];
	return value;
}
int popy() {
   	int value =stack[top][1];
	return value;
}
int FloodFill(int i,int j){
	cout<<image1.size().height<<endl;
	push(i,j);
	int ij[2];
	while(top>=0){
		ij[0] = popx();
		ij[1] = popy();
		top--;
 		if(ij[0]>=0&&ij[0]<image1.rows&&(ij[1])>=0&&(ij[1])<image1.cols&&image1.at<uchar>(ij[0],ij[1])==255){
			image1.at<uchar>(ij[0],ij[1])= label;
			push(ij[0]+1,ij[1]);
			push(ij[0],ij[1]+1);
			push(ij[0],ij[1]-1);
			push(ij[0]-1,ij[1]);
		}
	}
	return 0;
}
int Region_Labelling(){
	for(int i=0;i<image1.rows;i++){
		for(int j=0;j<image1.cols;j++){
			if(image1.at<uchar>(i,j)==255){
				label++;
				FloodFill(i,j);
			}
		}
	}
	return 0;
}
void contour_calc(){
	for(int i=0;i<label;i++){
		int number=0;
		for(int m=0;m<image1.size().height;m++){
			for(int n=0;n<image1.size().width;n++){
				if(image1.at<uchar>(m,n)==(i+1)){
					contours[i][number][0]=m;
					contours[i][number][1]=n;
					number++;
					int lastdir=3;
					int k=m;
					int l=n;
					int doo=1;
					while (k!=m||l!=n||doo==1){
						doo=0;
						lastdir=(lastdir+5)%8;
						for(int h=0;h<8;h++){
							if((lastdir+h)%8==0){
								if(image1.at<uchar>(k,l+1)==(i+1)){
									l+=1;
									contours[i][number][0]=k;
									contours[i][number][1]=l;
									number++;
									lastdir=0;
									h=8;
								}
							}else if((lastdir+h)%8==1){
								if(image1.at<uchar>(k+1,l+1)==(i+1)){
									k+=1;
									l+=1;
									contours[i][number][0]=k;
									contours[i][number][1]=l;
									number++;
									lastdir=1;
									h=8;
								}
							}else if((lastdir+h)%8==2){
								if(image1.at<uchar>(k+1,l)==(i+1)){
									k+=1;
									contours[i][number][0]=k;
									contours[i][number][1]=l;
									number++;
									lastdir=2;
									h=8;
								}
							}else if((lastdir+h)%8==3){
								if(image1.at<uchar>(k+1,l-1)==(i+1)){
									k+=1;
									l-=1;
									contours[i][number][0]=k;
									contours[i][number][1]=l;
									number++;
									lastdir=3;
									h=8;
								}
							}else if((lastdir+h)%8==4){
								if(image1.at<uchar>(k,l-1)==(i+1)){
									l-=1;
									contours[i][number][0]=k;
									contours[i][number][1]=l;
									number++;
									lastdir=4;
									h=8;
								}
							}else if((lastdir+h)%8==5){
								if(image1.at<uchar>(k-1,l-1)==(i+1)){
									k-=1;
									l-=1;
									contours[i][number][0]=k;
									contours[i][number][1]=l;
									number++;
									lastdir=5;
									h=8;
								}
							}else if((lastdir+h)%8==6){
								if(image1.at<uchar>(k-1,l)==(i+1)){
									k-=1;
									contours[i][number][0]=k;
									contours[i][number][1]=l;
									number++;
									lastdir=6;
									h=8;
								}
							}else if((lastdir+h)%8==7){
								if(image1.at<uchar>(k-1,l+1)==(i+1)){
									k-=1;
									l+=1;
									contours[i][number][0]=k;
									contours[i][number][1]=l;
									number++;
									lastdir=7;
									h=8;
								}
							}	
						}
					}
					m=image1.size().height;//This m and n values arranged for other loop
					n=image1.size().width;//
				}
			}
		}
		contours[i][number][0]=-1;
		contours[i][number][1]=-1;
	}
	
}
int main(){
	read_data("Database/MNIST/gaussnoisefiltered-images.idx3-ubyte", "Database/MNIST/t10k-labels.idx1-ubyte");
	
	image1=test_images[9999].clone();
	threshold(image1,image1, 127, 255, THRESH_BINARY);
	cv::imshow("1", image1);
	waitKey(0);
	Region_Labelling();
	contour_calc();
    
///////////////////////////////////////////////////////////////////////////////////////////////////
//////    PERIMATER (BOUNDARY LENGTH) CALCULATION   ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
	int perimater[label][2];
	float perimatertam[label];
	cout<< "Perimaters:"<<endl;	
	for(int i=0;i<label;i++){
		int tam=0,kok=0;
		for(int m=1;m<1000;m++){
			if(contours[i][m][0]!=-1&&contours[i][m][1]!=-1){
				int a1=contours[i][m-1][0];
				int b1=contours[i][m-1][1];
				int a2=contours[i][m][0];
				int b2=contours[i][m][1];
				if(((a2-a1==-1)&&(b2-b1==-1))||((a2-a1==-1)&&(b2-b1==1))||((a2-a1==1)&&(b2-b1==-1))||((a2-a1==1)&&(b2-b1==1))){
					kok++; 				
				}else{
					tam++;
				}
			}else{
				m=1000;
			}						
		}
		perimater[i][0]=tam;
		perimater[i][1]=kok;
		perimatertam[i]=(float)perimater[i][0]+((float)perimater[i][1]*sqrt(2));
		cout<<"L"<<i+1<<": "<<perimater[i][0]<<"+"<<perimater[i][1]<<"*2^½ "<<"= "<< perimatertam[i]<<endl;
	}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//////    AREA CALCULATION   //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
	int area[label]={0};
	for(int m=0;m<image1.rows;m++){
		for(int n=0;n<image1.cols;n++){
			for(int i=0;i<label;i++){
				if(image1.at<uchar>(m,n)==i+1){
					area[i]++;
				}
			}	
		}
	}
	
	/*cout<< "Areas:"<<endl;
	for(int i=0;i<label;i++){
		cout<<"L"<<i+1<<": "<<area[i]<<endl;
	}*/
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//////    COMPACTNESS CALCULATION   ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
	float compactness[label];	
	cout<< "Compactness:"<<endl;
	for(int i=0;i<label;i++){
		compactness[i]=((perimatertam[i]*perimatertam[i])/(float)area[i]);
		cout<<"L"<<i+1<<": "<<compactness[i]<<endl;
	}
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//////    Hu Moment CALCULATION   ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
	float Humoment[label][7];
	int sumofx[label]={0};
	int sumofy[label]={0};	
	float centeroidx[label]={0};
	float centeroidy[label]={0};
	float mu[label][4][4]={0};
	float normmu[label][4][4]={0};	
	cout<< "Hu Moments:"<<endl;
	for(int i=0;i<label;i++){
		for(int m=0;m<image1.rows;m++){
			for(int n=0;n<image1.cols;n++){
				if(image1.at<uchar>(m,n)==i+1){
					sumofx[i]+=m;
					sumofy[i]+=n;
				}
			}
		}
		centeroidx[i]=(float)sumofx[i]/area[i];
		centeroidy[i]=(float)sumofy[i]/area[i];
		for(int m=0;m<image1.rows;m++){
			for(int n=0;n<image1.cols;n++){
				if(image1.at<uchar>(m,n)==i+1){
					for(int a=0;a<4;a++){
						for(int b=0;b<4;b++){
							mu[i][a][b]+=pow((float)((float)m-centeroidx[i]),(float)a)*pow((float)((float)n-centeroidy[i]),(float)b);
						}
					}
				}
			}
		}
		for(int a=0;a<4;a++){
			for(int b=0;b<4;b++){
				normmu[i][a][b]=(float)mu[i][a][b]*pow(((float)1/area[i]),(float)((a+b+2)/2));
			}
		}
		Humoment[i][0]=normmu[i][2][0]+normmu[i][0][2];
		Humoment[i][1]=pow((float)(normmu[i][2][0]-normmu[i][0][2]),(float)2)+4*pow(normmu[i][1][1],(float)2);
		Humoment[i][2]=pow((float)(normmu[i][3][0]-(3*normmu[i][1][2])),(float)2)+pow((float)((3*normmu[i][2][1])-normmu[i][0][3]),(float)2);
		Humoment[i][3]=pow((float)(normmu[i][3][0]+normmu[i][1][2]),(float)2)+pow((float)(normmu[i][2][1]+normmu[i][0][3]),(float)2);
		Humoment[i][4]=(normmu[i][3][0]-3*normmu[i][1][2])*(normmu[i][3][0]+normmu[i][1][2])*(pow((float)(normmu[i][3][0]+normmu[i][1][2]),(float)2)-3*pow((float)(normmu[i][2][1]+normmu[i][0][3]),(float)2))+(((3*normmu[i][2][1])-normmu[i][0][3]))*((float)(normmu[i][2][1]+normmu[i][0][3]))*(3*pow((float)(normmu[i][3][0]+normmu[i][1][2]),(float)2)-pow((float)(normmu[i][2][1]+normmu[i][0][3]),(float)2));
		Humoment[i][5]=((float)(normmu[i][2][0]-normmu[i][0][2]))*((pow((float)(normmu[i][3][0]+normmu[i][1][2]),(float)2)-3*pow((float)(normmu[i][2][1]+normmu[i][0][3]),(float)2)))+4*normmu[i][1][1]*((float)(normmu[i][3][0]+normmu[i][1][2]))*((float)(normmu[i][2][1]+normmu[i][0][3]));
		Humoment[i][6]=(3*normmu[i][2][1]-normmu[i][0][3])*(normmu[i][3][0]+normmu[i][1][2])*(pow((float)(normmu[i][3][0]+normmu[i][1][2]),(float)2)-3*pow((float)(normmu[i][2][1]+normmu[i][0][3]),(float)2))+(((3*normmu[i][1][2])-normmu[i][3][0]))*((float)(normmu[i][2][1]+normmu[i][0][3]))*(3*pow((float)(normmu[i][3][0]+normmu[i][1][2]),(float)2)-pow((float)(normmu[i][2][1]+normmu[i][0][3]),(float)2));
		cout<<"L"<<i+1<<": "<<endl;
		cout<<"Hu Moment 1: "<<Humoment[i][0]<<endl;
		cout<<"Hu Moment 2: "<<Humoment[i][1]<<endl;
		cout<<"Hu Moment 3: "<<Humoment[i][2]<<endl;	
		cout<<"Hu Moment 4: "<<Humoment[i][3]<<endl;
		cout<<"Hu Moment 5: "<<Humoment[i][4]<<endl;
		cout<<"Hu Moment 6: "<<Humoment[i][5]<<endl;
		cout<<"Hu Moment 7: "<<Humoment[i][6]<<endl;
		cout<<"-----------------------------"<<endl;
	}
///////////////////////////////////////////////////////////////////////////////////////////////////

	return 0;
}



