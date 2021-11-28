#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

Mat image1;
int label=0;
int contours[100][1000][2];
int stack[1000][2], top=-1;
Mat test_images[10000];
char test_labels[10000];
float features [10000][9];// area, compactness, Hu moment
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

    image_file.read((char*)(&rows), 4);
    rows = reverseInt(rows);
    image_file.read((char*)(&cols), 4);
    cols = reverseInt(cols);

    char label;
    char* pixels = new char[rows * cols];
    for (int item_id = 0; item_id < num_items; ++item_id) {
        image_file.read(pixels, rows * cols);
        label_file.read(&label, 1);
		test_labels[item_id]=label;
        Mat image_tmp(rows,cols,CV_8U,pixels);
		test_images[item_id]=image_tmp.clone();
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
	label=0;
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
void writefeatures(int mode){
	const char* filename ;
	if(mode==1)
		filename = "Database/MNIST/testfeatures-gaussnoise.idx1-ubyte";	
	else if(mode==2)
		filename = "Database/MNIST/testfeatures-spnoise.idx1-ubyte";
	
	// Open files
	FILE* FP = fopen(filename,"wb");
	
	fwrite(features, 90000, sizeof(float), FP);
	fclose(FP);

}
void thresholdimage(int th){
	for(int a=0;a<28;a++){
		for(int b=0;b<28;b++){
			if(image1.at<uchar>(a,b)<th){
				image1.at<uchar>(a,b)=0;
			}else{
				image1.at<uchar>(a,b)=255;
			}
		}
	}
}
int main(){
	int mode=2;//Mode1 == gaussnoisefiltered images
			   //Mode2 == spnoisefiltered images
	const char* filename ;
	if(mode==1)
		filename = "Database/MNIST/gaussnoisefiltered-images.idx3-ubyte";	
	else if(mode==2)
		filename = "Database/MNIST/spnoisefiltered-images.idx3-ubyte";
	read_data(filename, "Database/MNIST/t10k-labels.idx1-ubyte");
	Mat image2;
	waitKey(0);	
	for(int ioimg=0;ioimg<10000;ioimg++){
	image1=test_images[ioimg].clone();
	thresholdimage(100);
	Region_Labelling();
	contour_calc();
    int correctlabel=0;
	float correctlabelval=0;
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
		if(perimatertam[i]>correctlabelval){
			correctlabelval= perimatertam[i];
			correctlabel=i;
		}		
	}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//////    AREA CALCULATION   //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
	int area=0;
	for(int m=0;m<image1.rows;m++){
		for(int n=0;n<image1.cols;n++){
			if(image1.at<uchar>(m,n)==correctlabel+1){
				area++;
			}
		}
	}
	
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//////    COMPACTNESS CALCULATION   ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
	float compactness;	
	compactness=((perimatertam[correctlabel]*perimatertam[correctlabel])/(float)area);
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//////    Hu Moment CALCULATION   ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
	float Humoment[7];
	int sumofx=0;
	int sumofy=0;	
	float centeroidx=0;
	float centeroidy=0;
	float mu[4][4]={0};
	float normmu[4][4]={0};	
	for(int m=0;m<image1.rows;m++){
		for(int n=0;n<image1.cols;n++){
			if(image1.at<uchar>(m,n)==correctlabel+1){
					sumofx+=m;
					sumofy+=n;
			}
		}
	}
	centeroidx=(float)sumofx/area;
	centeroidy=(float)sumofy/area;
	for(int m=0;m<image1.rows;m++){
		for(int n=0;n<image1.cols;n++){
			if(image1.at<uchar>(m,n)==correctlabel+1){
				for(int a=0;a<4;a++){
					for(int b=0;b<4;b++){
						mu[a][b]+=pow((float)((float)m-centeroidx),(float)a)*pow((float)((float)n-centeroidy),(float)b);
					}
				}
			}
		}
	}
	for(int a=0;a<4;a++){
		for(int b=0;b<4;b++){
			normmu[a][b]=(float)mu[a][b]*pow(((float)1/area),(float)((a+b+2)/2));
		}
	}
	Humoment[0]=normmu[2][0]+normmu[0][2];
	Humoment[1]=pow((float)(normmu[2][0]-normmu[0][2]),(float)2)+4*pow(normmu[1][1],(float)2);
	Humoment[2]=pow((float)(normmu[3][0]-(3*normmu[1][2])),(float)2)+pow((float)((3*normmu[2][1])-normmu[0][3]),(float)2);
	Humoment[3]=pow((float)(normmu[3][0]+normmu[1][2]),(float)2)+pow((float)(normmu[2][1]+normmu[0][3]),(float)2);
	Humoment[4]=(normmu[3][0]-3*normmu[1][2])*(normmu[3][0]+normmu[1][2])*(pow((float)(normmu[3][0]+normmu[1][2]),(float)2)-3*pow((float)(normmu[2][1]+normmu[0][3]),(float)2))+(((3*normmu[2][1])-normmu[0][3]))*((float)(normmu[2][1]+normmu[0][3]))*(3*pow((float)(normmu[3][0]+normmu[1][2]),(float)2)-pow((float)(normmu[2][1]+normmu[0][3]),(float)2));
	Humoment[5]=((float)(normmu[2][0]-normmu[0][2]))*((pow((float)(normmu[3][0]+normmu[1][2]),(float)2)-3*pow((float)(normmu[2][1]+normmu[0][3]),(float)2)))+4*normmu[1][1]*((float)(normmu[3][0]+normmu[1][2]))*((float)(normmu[2][1]+normmu[0][3]));
	Humoment[6]=(3*normmu[2][1]-normmu[0][3])*(normmu[3][0]+normmu[1][2])*(pow((float)(normmu[3][0]+normmu[1][2]),(float)2)-3*pow((float)(normmu[2][1]+normmu[0][3]),(float)2))+(((3*normmu[1][2])-normmu[3][0]))*((float)(normmu[2][1]+normmu[0][3]))*(3*pow((float)(normmu[3][0]+normmu[1][2]),(float)2)-pow((float)(normmu[2][1]+normmu[0][3]),(float)2));
	
///////////////////////////////////////////////////////////////////////////////////////////////////
	features[ioimg][0]=area;
	features[ioimg][1]=compactness;
	features[ioimg][2]=Humoment[0];
	features[ioimg][3]=Humoment[1];
	features[ioimg][4]=Humoment[2];
	features[ioimg][5]=Humoment[3];
	features[ioimg][6]=Humoment[4];
	features[ioimg][7]=Humoment[5];
	features[ioimg][8]=Humoment[6];
	
	cout<<"Image "<< ioimg<< " Area: " <<features[ioimg][0]<<endl;
	cout<<"Image "<< ioimg<< " Compactness: " <<features[ioimg][1]<<endl;
	cout<<"Image "<< ioimg<< " Hu 1: " <<features[ioimg][2]<<endl;
	cout<<"Image "<< ioimg<< " Hu 2: " <<features[ioimg][3]<<endl;
	cout<<"Image "<< ioimg<< " Hu 3: " <<features[ioimg][4]<<endl;
	cout<<"Image "<< ioimg<< " Hu 4: " <<features[ioimg][5]<<endl;
	cout<<"Image "<< ioimg<< " Hu 5: " <<features[ioimg][6]<<endl;
	cout<<"Image "<< ioimg<< " Hu 6: " <<features[ioimg][7]<<endl;
	cout<<"Image "<< ioimg<< " Hu 7: " <<features[ioimg][8]<<endl;
	cout<< "------------------------------------" <<endl;
}
	writefeatures(mode);
	return 0;
}



