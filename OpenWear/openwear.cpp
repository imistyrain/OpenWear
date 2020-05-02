#include "mrgl.h"
#include "mropencv.h"
#include "Glasses.h"
#include "ldmarkmodel.h"

const std::string modeldir = "model/";
VideoCapture capture(0);
cv::Mat img;
CGlasses m_glasses;
ldmarkmodel modelt;
std::string modelFilePath = modeldir+"roboman-landmark-model.bin";
cv::Mat current_shape;
GLuint textureID;

float smoothingeyecenterx = 0;
float smoothingeyecentery = 0;
float smoothingscale = 1.1;
float learningrate = 0.2;
const char* strlogo = "OpenWear 1.0";

void initOpenGL(void){
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(300, 200);
	glutCreateWindow(strlogo);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	float light_pos[4];
	light_pos[2] = -2.0;
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glEnable(GL_LIGHT0);
	m_glasses.ReadData();
	if(!load_ldmarkmodel(modelFilePath, modelt)){
		std::cout << "Failed to load sdm model" << std::endl;
		exit(-1);
	}
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void drawVideo(const cv::Mat &img){
	flip(img, img, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-3.2, -2.4, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-3.2, 2.4, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(3.2, 2.4, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(3.2, -2.4, 0.0);
	glEnd();
}

void display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	capture >> img;
	if (!img.data)
		return;
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -4);
	glPushMatrix();
	modelt.track(img, current_shape);
	if (current_shape.cols == 136)
	{
		cv::Vec3d eav;		
		modelt.drawPose(img, current_shape, eav);
		cv::Point lefteye = cv::Point((current_shape.at<float>(36) + current_shape.at<float>(39)) / 2,
			(current_shape.at<float>(36 + 68) + current_shape.at<float>(39 + 68)) / 2);
		cv::Point righteye = cv::Point((current_shape.at<float>(42) + current_shape.at<float>(45))/2,
			(current_shape.at<float>(42 + 68) + current_shape.at<float>(45 + 68)) / 2);
		cv::circle(img, lefteye, 2, cv::Scalar(0, 255, 0), -1);
		cv::circle(img, righteye, 2, cv::Scalar(0, 255, 0), -1);
		float eyecenterx = (lefteye.x + righteye.x) / 2;
		float eyecentery = (lefteye.y + righteye.y) / 2;
		smoothingeyecenterx = smoothingeyecenterx*(1 - learningrate) + learningrate*eyecenterx;
		smoothingeyecentery = smoothingeyecentery*(1 - learningrate) + learningrate*eyecentery;
		float distance = cv::norm(lefteye-righteye);
		float scale = 0.008*distance + 0.24;
		smoothingscale = learningrate*scale + (1 - learningrate)*smoothingscale;
		glTranslatef((smoothingeyecenterx - 320) / 320 * 2, (240 - smoothingeyecentery) / 240 * 2, 0);
		glRotatef(0.5*eav[0], 1, 0,0);
 		glRotatef(-0.5*eav[1], 0, 1, 0);
 		glRotatef(-0.5*eav[2], 0, 0, 1);
		glScalef(smoothingscale, smoothingscale, smoothingscale);		
	}
	m_glasses.Draw();
	glPopMatrix();
	glPushMatrix();
	drawVideo(img);
	glPopMatrix();	
	glutSwapBuffers();
	glutPostRedisplay();
}

void reshape(int w, int h){
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, w * 1.0 / h, 1.0, 30.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char *argv[]){
	glutInit(&argc, argv);
	initOpenGL();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;
}