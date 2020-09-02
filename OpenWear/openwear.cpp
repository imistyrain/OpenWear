#define GL_SILENCE_DEPRECATION

#include "mrgl.h"
#include "Glasses.h"
#include "ldmarkmodel.h"
#include "mrutil.h"
#include "fstream"
#include "mropencv.h"

const std::string modeldir = "model";
VideoCapture capture(0);
cv::Mat img;
int frame_width = 640;
int frame_height = 480;

GLuint textureID;
CGlasses m_glasses;
ldmarkmodel modelt;
std::string modelFilePath = modeldir+"/roboman-landmark-model.bin";
cv::Mat current_shape;

const char* strlogo = "OpenWear 1.0";
const float dist = 300.0f;
float g_scale = 160.0f;
bool g_bshowlandmarks = true;
bool g_bshowtext = true;
float matMV[16];

void on_timer(int val){
	glutPostRedisplay();
	glutTimerFunc(10,on_timer,0);
}

void reshape(int w, int h){
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, w * 1.0f / h, 0.1, 400);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void drawVideo(cv::Mat &img){
	flip(img, img, 0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	float aspect = img.cols * 1.0 / img.rows;
	glLoadIdentity();
	glTranslatef(0,0,-dist);
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(-aspect*dist, -dist, 0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-aspect*dist, dist, 0);
	glTexCoord2f(1.0, 1.0); glVertex3f(aspect*dist, dist, 0);
	glTexCoord2f(1.0, 0.0); glVertex3f(aspect*dist, -dist, 0);
	glEnd();
}

void processNormalKeys(unsigned char key,int x,int y){
    switch(key){
		case 's':
		case 'S':
			g_bshowlandmarks = !g_bshowlandmarks;
			break;
		case 't':
		case 'T':
			g_bshowtext = !g_bshowtext;
			break;
	}
}

void processSpecialKey(GLint key, GLint x, GLint y) {
    if (key == GLUT_KEY_LEFT){
        g_scale *= 1.05f;
    }
    if (key == GLUT_KEY_RIGHT){
        g_scale /= 1.05f;
    }
}

void drawLandmark(cv::Mat& img, const cv::Mat& current_shape,bool showText = true){
	int numLandmarks = current_shape.cols / 2;
	for (int j = 0; j < numLandmarks; j++){
		int x = current_shape.at<float>(j);
		int y = current_shape.at<float>(j + numLandmarks);
		cv::circle(img, cv::Point(x, y), 2, {255, 0, 0}, -1);
		if (showText){
			cv::putText(img,int2string(j),cv::Point(x, y),1,1,{0,0,255});
		}
	}
}

void drawArrow(cv::Mat image, cv::Point2i p, cv::Point2i q, cv::Scalar color,  int arrowMagnitude = 9, int thickness=1, int line_type=8, int shift=0)
{
    //Draw the principle line
    cv::line(image, p, q, color, thickness, line_type, shift);
    const double PI = CV_PI;
    //compute the angle alpha
    double angle = atan2((double)p.y-q.y, (double)p.x-q.x);
    //compute the coordinates of the first segment
    p.x = (int) ( q.x +  arrowMagnitude * cos(angle + PI/4));
    p.y = (int) ( q.y +  arrowMagnitude * sin(angle + PI/4));
    //Draw the first segment
    cv::line(image, p, q, color, thickness, line_type, shift);
    //compute the coordinates of the second segment
    p.x = (int) ( q.x +  arrowMagnitude * cos(angle - PI/4));
    p.y = (int) ( q.y +  arrowMagnitude * sin(angle - PI/4));
    //Draw the second segment
    cv::line(image, p, q, color, thickness, line_type, shift);
}
void draw3DCoordinateAxes(cv::Mat image, const std::vector<cv::Point2f> &list_points2d)
{
    cv::Scalar red(0, 0, 255);
    cv::Scalar green(0,255,0);
    cv::Scalar blue(255,0,0);
    cv::Scalar black(0,0,0);

    cv::Point2i origin = list_points2d[0];
    cv::Point2i pointX = list_points2d[1];
    cv::Point2i pointY = list_points2d[2];
    cv::Point2i pointZ = list_points2d[3];

    drawArrow(image, origin, pointX, red, 9, 2);
    drawArrow(image, origin, pointY, green, 9, 2);
    drawArrow(image, origin, pointZ, blue, 9, 2);
    cv::circle(image, origin, 2, black, -1 );
}
void drawEAV(cv::Mat &img,cv::Vec3f &eav){
	cv::putText(img,"x:"+double2string(eav[0]),cv::Point(0,30),3,1,cv::Scalar(0,0,255));
	cv::putText(img,"y:"+double2string(eav[1]),cv::Point(0,60),3,1,cv::Scalar(0,255,0));
	cv::putText(img,"z:"+double2string(eav[2]),cv::Point(0,90),3,1,cv::Scalar(255,0,0));
}

void estimatePose(cv::Mat &img, cv::Mat &current_shape){
	cv::Mat rvec, tvec;
	std::vector<cv::Point3f > modelPoints;
	modelPoints.push_back(Point3f(2.37427, 110.322, 21.7776));	// l eye (v 314)
    modelPoints.push_back(Point3f(70.0602, 109.898, 20.8234));	// r eye (v 0)
    modelPoints.push_back(Point3f(36.8301, 78.3185, 52.0345));	//nose (v 1879)
    modelPoints.push_back(Point3f(14.8498, 51.0115, 30.2378));	// l mouth (v 1502)
    modelPoints.push_back(Point3f(58.1825, 51.0115, 29.6224));	// r mouth (v 695) 
	int max_d = (img.rows + img.cols)/2;
	cv::Mat camMatrix = (Mat_<double>(3, 3) << max_d, 0, img.cols / 2.0,0, max_d, img.rows / 2.0,0, 0, 1.0);
	std::vector<cv::Point2f > imagePoints;
	int ptindexes[] = {36,39,42,45,30,48,54};
	int numLandmarks = current_shape.cols / 2;
	for (int i=0;i<5;i++){
		int x,y;
		if (i < 2){
			int pt1 = ptindexes[2*i];
			int pt2 = ptindexes[2*i+1];
			x = (current_shape.at<float>(pt1)+current_shape.at<float>(pt2))/2;
			y = (current_shape.at<float>(pt1+numLandmarks)+current_shape.at<float>(pt2+numLandmarks))/2;
		} else {
			int pt = ptindexes[i+2];
			x = current_shape.at<float>(pt);
			y = current_shape.at<float>(pt+numLandmarks);
		}
		imagePoints.push_back(cv::Point(x,y));
		cv::circle(img,cv::Point(x,y),3,{255,0,0},-1);
	}
    solvePnP(modelPoints,imagePoints, camMatrix, cv::Mat(), rvec, tvec, false, CV_EPNP);
	cv::Mat rotM;
	cv::Rodrigues(rvec, rotM);
	static double d[] ={
		1, 0, 0,
		0, -1, 0,
		0, 0, -1
	};
	Mat_<double> rx(3, 3, d);
    cv::Mat rotation = rx * rotM;
	cv::Mat translation = rx * tvec;
	std::vector<cv::Point3f> axises;
	std::vector<cv::Point2f> pts2d;
	float l = 40;
	int x = modelPoints[2].x;
	int y = modelPoints[2].y;
	int z = modelPoints[2].z;
	axises.push_back(cv::Point3f(x,y,z));
	axises.push_back(cv::Point3f(x+l,y,z));
	axises.push_back(cv::Point3f(x,y+l,z));
	axises.push_back(cv::Point3f(x,y,z+l));
	projectPoints(axises,rvec,tvec,camMatrix,cv::Mat(),pts2d);
	draw3DCoordinateAxes(img,pts2d);
	cv:Mat T;
	cv::Mat euler_angle;
	cv::Mat out_rotation, out_translation;
	cv::hconcat(rotM, rvec, T);
	cv::decomposeProjectionMatrix(T,camMatrix,out_rotation,out_translation,cv::noArray(),cv::noArray(),cv::noArray(),euler_angle);
	cv::Vec3f eav;
	for(int i = 0; i < 3; i++){
		eav[i] = euler_angle.at<double>(0,i);
	}
	drawEAV(img,eav);
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 3; j++){
			matMV[i*4+j] = rotation.at<double>(j,i);
		}
		matMV[12+i] = translation.at<double>(i,0);
	}
	matMV[15] = 1.0f;
}

void show_matrix(bool colmajor = false){
    if (colmajor){
        cv::Mat cvmatMV = cv::Mat(4,4,CV_32FC1, matMV);
        std::cout << "MV" << std::endl;
        std::cout << cvmatMV << std::endl;
    } else {
        std::cout.precision(2);
        std::cout.width(8);
        std::cout<<"\t\tMV";
        std::cout<<std::endl;
        for (int i = 0;i < 4; i++){
            for (int j = 0;j < 4; j++){
                std::cout<<matMV[i+4*j]<<"\t";
            }
            std::cout<<std::endl;
        }
        std::cout.precision(8);
    }
}

void display(void){
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	capture >> img;
	if (!img.data)
		return;
	
	modelt.track(img, current_shape);
	cv::Vec3d eav;
	if (current_shape.cols == 136){
		if (g_bshowlandmarks){
			drawLandmark(img, current_shape,g_bshowtext);
		}
		estimatePose(img,current_shape);
		
	}
	glPushMatrix();
		drawVideo(img);
	glPopMatrix();

	glPushMatrix();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glLoadMatrixf(matMV);
		glTranslatef(40,60,-2);
		glScalef(g_scale,g_scale,g_scale);
		//show_matrix();
		m_glasses.Draw();
	glPopMatrix();
	glutSwapBuffers();
	#if _WIN32
		glutPostRedisplay();
	#endif
}

void initOpenGL(void){
	capture >> img;
	if (!img.data){
		std::cout<<"Failed to open camera "<<std::endl;
		exit(-1);
	}
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	frame_width = img.cols;
	frame_height = img.rows;
	glutInitWindowSize(frame_width, frame_height);
	glutCreateWindow(strlogo);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_LIGHTING);
	float light_pos[4];
	light_pos[2] = -dist + 2.0f;
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glEnable(GL_LIGHT0);
	m_glasses.ReadData();
	if(!load_ldmarkmodel(modelFilePath, modelt)){
		std::cout << "Failed to load sdm model" << std::endl;
		exit(-2);
	}
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKey);
	glutTimerFunc(30,on_timer,0);
}

int main(int argc, char *argv[]){
	glutInit(&argc, argv);
	initOpenGL();
	glutMainLoop();
	return 0;
}