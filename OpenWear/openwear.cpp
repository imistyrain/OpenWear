#define GL_SILENCE_DEPRECATION
#include "mrgl.h"
#include "mropencv.h"
#include "Glasses.h"
#include "ldmarkmodel.h"

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

float smoothingeyecenterx = 0;
float smoothingeyecentery = 0;
float smoothingscale = 1.1;
float learningrate = 0.2;
const char* strlogo = "OpenWear 1.0";
const float dist = 4.0f;
float scale = 1.0f;

void on_timer(int val){
	glutPostRedisplay();
	glutTimerFunc(20,on_timer,0);
}

void reshape(int w, int h){
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, w * 1.0f / h, 0.1, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void drawVideo(cv::Mat &img){
	flip(img, img, 0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	float aspect = img.cols * 1.0 / img.rows;
	glLoadIdentity();
	glTranslatef(0,0,-dist*scale);
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(-aspect*dist, -dist, 0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-aspect*dist, dist, 0);
	glTexCoord2f(1.0, 1.0); glVertex3f(aspect*dist, dist, 0);
	glTexCoord2f(1.0, 0.0); glVertex3f(aspect*dist, -dist, 0);
	glEnd();
}

void processSpecialKey(GLint key, GLint x, GLint y) {
    if (key == GLUT_KEY_LEFT){
        scale += 0.05f;
    }
    if (key == GLUT_KEY_RIGHT){
        scale -= 0.05f;
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
		modelt.drawPose(img, current_shape, eav);
		cv::Point lefteye = cv::Point((current_shape.at<float>(36) + current_shape.at<float>(39)) / 2,
			(current_shape.at<float>(36 + 68) + current_shape.at<float>(39 + 68)) / 2);
		cv::Point righteye = cv::Point((current_shape.at<float>(42) + current_shape.at<float>(45))/2,
			(current_shape.at<float>(42 + 68) + current_shape.at<float>(45 + 68)) / 2);
		float eyecenterx = (lefteye.x + righteye.x) / 2;
		float eyecentery = (lefteye.y + righteye.y) / 2;
		smoothingeyecenterx = smoothingeyecenterx*(1 - learningrate) + learningrate*eyecenterx;
		smoothingeyecentery = smoothingeyecentery*(1 - learningrate) + learningrate*eyecentery;
		float distance = cv::norm(lefteye-righteye);
		float scale = 0.008*distance + 0.24;
		smoothingscale = learningrate*scale + (1 - learningrate)*smoothingscale;
	}
	glPushMatrix();
		drawVideo(img);
	glPopMatrix();

	glPushMatrix();
		glLoadIdentity();
		float x = (smoothingeyecenterx / frame_width - 0.5)*dist;
		float y = (0.5 - smoothingeyecentery / frame_height)*dist;
		float z = -dist+0.5f;
		std::cout<<"\r"<<x<<","<<y<<","<<z<<std::flush;
		glTranslatef(x, y, z);
		glRotatef(0.5*eav[0], 1, 0, 0);
		glRotatef(-0.5*eav[1], 0, 1, 0);
		glRotatef(-0.5*eav[2], 0, 0, 1);
		glScalef(smoothingscale, smoothingscale, smoothingscale);
		m_glasses.Draw();
	glPopMatrix();

	glutSwapBuffers();
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
	glutSpecialFunc(processSpecialKey);
	glutTimerFunc(30,on_timer,0);
}

int main(int argc, char *argv[]){
	glutInit(&argc, argv);
	initOpenGL();
	glutMainLoop();
	return 0;
}