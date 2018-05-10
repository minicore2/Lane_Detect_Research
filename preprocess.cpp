#include "preprocess.hpp"

//constructor
void preprocess::process(Mat &input, int LowH, int HighH, int LowS, int HighS, int LowV, int HighV)
{
    this->LowH = LowH;
    this->HighH = HighH;
    //smaller S lighter it can go
    this->LowS = LowS;
    this->HighS = HighS;
    //smaller V blacker it can go
    this->LowV = LowV;
    this->HighV = HighV;
    //resize the image
    resize(input, input, Size(1280,720));
    image = input(Rect(0, input.rows / 2, input.cols, input.rows / 2 - 50));
    //image = image(Rect(0, image.rows / 5, image.cols, image.rows / 5 * 4 ));
    origin_image = IPM(image);
    //
    filter();
    //
    toHSV();
    //
    toBinary();
    //
    image = IPM(image);
    //
    refine();
    //-----------show image----------
    
#ifdef DEBUG_MODE
    //namedWindow("prep output");
    //imshow("prep output", image);
    //waitKey(0);
#endif
    
}

//to HSV, make it easier to set up threshold
void preprocess::toHSV()
{
    Mat imgHSV;
    vector<Mat> hsvSplit;
    //Convert the captured frame from BGR to HSV
    cvtColor(image, imgHSV, COLOR_BGR2HSV);
    split(imgHSV, hsvSplit);
    equalizeHist(hsvSplit[2],hsvSplit[2]);
    merge(hsvSplit,imgHSV);
    image = imgHSV;
}

//Gaussian filter to reduce noise casued by light spot
void preprocess::filter()
{
    Mat bluredImg;
    //GaussianBlur(image, bluredImg, Size( 2, 2 ), 0, 0 );
    blur(image, bluredImg, Size( 5, 5 ));
    image=bluredImg;
}

//change the image into Bird-eye view
Mat preprocess::IPM(Mat input)
{
    Point2f inputQuad[4];
    // Output Quadilateral or World plane coordinates
    Point2f outputQuad[4];
    
    // Lambda Matrix
    Mat lambda( 2, 4, CV_32FC1 );
    //Input and Output Image;
    Mat output;
    int coValue=0;
    int value = 286*2;
    if(value<0){
        coValue=-value;
    }
    // Set the lambda matrix the same type and size as input
    lambda = Mat::zeros( input.rows, input.cols, input.type() );
    
    // The 4 points that select quadilateral on the input , from top-left in clockwise order
    // These four pts are the sides of the rect box used as input
    inputQuad[0] = Point2f( 0,0 );
    inputQuad[1] = Point2f( input.cols,0);
    inputQuad[2] = Point2f( input.cols-coValue,input.rows+1);
    inputQuad[3] = Point2f( +coValue,input.rows+1  );
    // The 4 points where the mapping is to be done , from top-left in clockwise order
    outputQuad[0] = Point2f( 0,0 );
    outputQuad[1] = Point2f( input.cols-1,0);
    outputQuad[2] = Point2f( input.cols-value,input.rows-1);
    outputQuad[3] = Point2f( value,input.rows-1  );
    
    // Get the Perspective Transform Matrix i.e. lambda
    lambda = getPerspectiveTransform( inputQuad, outputQuad );
    // Apply the Perspective Transform just found to the src image
    warpPerspective(input,output,lambda,output.size() );
    return output;
}

//use threshold to change into binary image
void preprocess::toBinary()
{
    Mat imgThresholded;
    
    //Threshold the image
    inRange(image, Scalar(LowH, LowS, LowV), Scalar(HighH, HighS, HighV), imgThresholded);
    Mat element = getStructuringElement(MORPH_RECT, Size(1, 1));
    
    //open morph
    morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);
    
    //close morph
    morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);
    image=imgThresholded;
}

//change the threshold based on output
void preprocess::refine()
{
}

//return preprocessed result
Mat preprocess::prep_result()
{
    return image;
}
