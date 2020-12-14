#include "Render.h"


#include <future>
#include <sstream>
#include <iostream>
#include <fstream>


#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h"
#include <chrono>




GuiTextRectangle rec;

bool textureMode = true;
bool lightMode = true;


//��������� ������ ��� ��������� ����
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //��������� ��� ������ ��������
Shader frac;
Shader cassini;

double x = 0;
double y = 0;
double z = 0;
int a, b, c, d = 0;
long int angle1 = 0;
long int time1 = 0;

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	virtual void SetUpCamera()
	{

		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ���������!
class WASDcamera :public CustomCamera
{
public:
		
	float camSpeed;

	WASDcamera()
	{
		camSpeed = 0.4;
		pos.setCoords(5, 5, 5);
		lookPoint.setCoords(0, 0, 0);
		normal.setCoords(0, 0, 1);
	}

	virtual void SetUpCamera()
	{

		if (OpenGL::isKeyPressed('W'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*camSpeed;
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}
		if (OpenGL::isKeyPressed('S'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*(-camSpeed);
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}

		LookAt();
	}

} WASDcam;


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);
		
		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
				glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}
		/*
		if (f1)
			glEnable(GL_LIGHTING);
		if (f2)
			glEnable(GL_TEXTURE_2D);
		if (f3)
			glEnable(GL_DEPTH_TEST);
			*/
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 1, 1, 1, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����



//������ ���������� ����
int mouseX = 0, mouseY = 0;




float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//���������� �������� ����
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0*dx/ogl->getWidth()/zoom;
		offsetY += 1.0*dy/ogl->getHeight()/zoom;
	}


	
	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y,60,ogl->aspect);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

//���������� �������� ������  ����
void mouseWheelEvent(OpenGL *ogl, int delta)
{


	float _tmpZ = delta*0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;
}
bool rezh = true;
//���������� ������� ������ ����������
void keyDownEvent(OpenGL *ogl, int key)
{
	if (OpenGL::isKeyPressed('O'))
	{
		rezh=!rezh;
	}

	if (OpenGL::isKeyPressed('L'))
	{
		lightMode = !lightMode;
	}

	if (OpenGL::isKeyPressed('T'))
	{
		textureMode = !textureMode;
	}	   

	if (OpenGL::isKeyPressed('R'))
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (OpenGL::isKeyPressed('F'))
	{
		light.pos = camera.pos;
	}

	if (OpenGL::isKeyPressed('P'))
	{
		frac.LoadShaderFromFile();
		frac.Compile();

		s[0].LoadShaderFromFile();
		s[0].Compile();

		cassini.LoadShaderFromFile();
		cassini.Compile();
	}

	if (key == 'Q')
		Time = 0;

	
}

void mykeyevent()
{
	
	if (OpenGL::isKeyPressed('W'))
	{
		x += 0.4;
		a = 1;
		//time1 = 0;
		//ISoundEngine* SoundEngine = createIrrKlangDevice();
		//SoundEngine->play2D("mus.mp3", true);
	}
	else
		a = 0;

	if (OpenGL::isKeyPressed('S'))
	{
		x -= 0.4;
		b = 1;
	}
	else
		b = 0;

	if (OpenGL::isKeyPressed('A'))
	{
		y += 0.4;
		c = 1;
	}
	else
		c = 0;

	if (OpenGL::isKeyPressed('D'))
	{
		y -= 0.4;
		d = 1;
	}
	else
		d = 0;

	if (OpenGL::isKeyPressed('Y'))
		z += 0.4;

	if (OpenGL::isKeyPressed('H'))
		z -= 0.4;
}

void keyUpEvent(OpenGL *ogl, int key)
{
	if (key == 'W')
		a = 0;
	if (key == 'S')
		b = 0;
	if (key == 'A')
		c = 0;
	if (key == 'D')
		d = 0;
}


void DrawQuad()
{
	double A[] = { 0,0 };
	double B[] = { 1,0 };
	double C[] = { 1,1 };
	double D[] = { 0,1 };
	glBegin(GL_QUADS);
	glColor3d(.5, 0, 0);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);
	glEnd();
}


ObjFile objModel,monkey,cow,cow1, cow2, plosk,dom,dom2;

Texture monkeyTex, cityTex,cowTex,cow1Tex, cow2Tex, ploskTex,domTex, dom2Tex;

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{

	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	
	


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   �������� �������� �� �����
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	frac.FshaderFileName = "shaders\\frac.frag"; //��� ����� ������������ �������
	frac.LoadShaderFromFile(); //��������� ������� �� �����
	frac.Compile(); //�����������

	cassini.VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	cassini.FshaderFileName = "shaders\\cassini.frag"; //��� ����� ������������ �������
	cassini.LoadShaderFromFile(); //��������� ������� �� �����
	cassini.Compile(); //�����������
	

	s[0].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[0].FshaderFileName = "shaders\\light.frag"; //��� ����� ������������ �������
	s[0].LoadShaderFromFile(); //��������� ������� �� �����
	s[0].Compile(); //�����������

	s[1].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //��� ����� ������������ �������
	s[1].LoadShaderFromFile(); //��������� ������� �� �����
	s[1].Compile(); //�����������

	

	 //��� ��� ��� ������� ������ *obj_m �����, ��� ��� ��� ��������� �� ���������� � ���������� �������, 
	 // ������������ �� ����� ����������, � ������������ ������ � *obj_m_m
	//loadModel("models\\planeobj_m", &objModel);


	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\Spaceshipobj_m", &monkey);
	monkeyTex.loadTextureFromFile("textures//Ship_Base_Color.bmp");
	monkeyTex.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\cowobj_m", &cow);
	cowTex.loadTextureFromFile("textures//cow.bmp");
	cowTex.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\cow1obj_m", &cow1);
	cow1Tex.loadTextureFromFile("textures//cow.bmp");
	cow1Tex.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\cow2obj_m", &cow2);
	cow2Tex.loadTextureFromFile("textures//cow.bmp");
	cow2Tex.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\ploskobj_m", &plosk);
	ploskTex.loadTextureFromFile("textures//plosk.bmp");
	ploskTex.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\domobj_m", &dom);
	domTex.loadTextureFromFile("textures//dom.bmp");
	domTex.bindTexture();

	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\dom2obj_m", &dom2);
	dom2Tex.loadTextureFromFile("textures//dom2.bmp");
	dom2Tex.bindTexture();

	


	tick_n = GetTickCount();
	tick_o = tick_n;

	/*GuiTextRectangle rec;		  
	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 200 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "O - ������ �������� ���" << std::endl;
	ss << "W,A,S,D,Y,H - ���������� ���" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "CTRL+S ��������� ������� ��������� �����������" << std::endl;
	ss << "CTRL+U ��������� ����������� �� �����" << std::endl;
	ss << "�����. �����: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "�����. ������: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "��������� ������: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

	rec.setText(ss.str().c_str());*/

	
}

int num = 0;
double f(double p1, double p2, double p3, double t)
{
	return p1 * (1 - t) * (1 - t) * (1 - t) + 2 * p2 * t * (1 - t) + p3 * t * t; //����������� �������
}
double f(double p1, double p2, double p3, double p4, double t)
{
	return p1 * (1 - t) * (1 - t) * (1 - t) + 3 * p2 * t * (1 - t) * (1 - t) + 3 * p3 * t * t * (1 - t) + p4 * t * t * t; //����������� �������
}
Vector3 bizeWithoutDraw(double P1[3], double P2[3], double P3[3], double P4[3], double t)
{
	Vector3 Vec;
	Vec.setCoords(f(P1[0], P2[0], P3[0], P4[0], t), f(P1[1], P2[1], P3[1], P4[1], t), f(P1[2], P2[2], P3[2], P4[2], t));
	return Vec;
}
void Bese2(double P1[3], double P2[3], double P3[3], double P4[3], double delta_time)
{
	static double t_max = 0;
	static bool flagReverse = false;

	if (!flagReverse)
	{
		t_max += delta_time / 5; //t_max ���������� = 1 �� 5 ������
		if (t_max > 1)
		{
			t_max = 1; //����� ����������
			flagReverse = !flagReverse;
		}
	}
	else
	{
		t_max -= delta_time / 5; //t_max ���������� = 1 �� 5 ������
		if (t_max < 0)
		{
			t_max = 0; //����� ����������
			flagReverse = !flagReverse;
		}
	}

	


	Vector3 P_old = bizeWithoutDraw(P1, P2, P3, P4, !flagReverse ? t_max - delta_time : t_max + delta_time);
	Vector3 P = bizeWithoutDraw(P1, P2, P3, P4, t_max);
	Vector3 VecP_P_old = (P - P_old).normolize();

	Vector3 rotateX(VecP_P_old.X(), VecP_P_old.Y(), 0);
	rotateX = rotateX.normolize();

	Vector3 VecPrX = Vector3(1, 0, 0).vectProisvedenie(rotateX);
	double CosX = Vector3(1, 0, 0).ScalarProizv(rotateX);
	double SinAngleZ = VecPrX.Z() / abs(VecPrX.Z());
	double AngleOZ = acos(CosX) * 180 / PI * SinAngleZ;

	double AngleOY = acos(VecP_P_old.Z()) * 180 / PI - 90;

	double A[] = { -0.5,-0.5,-0.5 };
	glPushMatrix();
	glTranslated(P.X(), P.Y(), P.Z());
	glRotated(AngleOZ, 0, 0, 1);
	glRotated(AngleOY, 0, 1, 0);
	
	glRotated(angle1, 0, 0, 1);
	monkeyTex.bindTexture();
	monkey.DrawObj();
	glPopMatrix();

	glColor3d(0, 0, 0);

}
static double Search_delta_time() 
{
	static auto end_render = std::chrono::steady_clock::now();
	auto cur_time = std::chrono::steady_clock::now();
	auto deltatime = cur_time - end_render;
	double delta = 1.0 * std::chrono::duration_cast<std::chrono::microseconds>(deltatime).count() / 1000000;
	end_render = cur_time;
	return delta;
}

void Render(OpenGL *ogl)
{   
	
	mykeyevent();

	tick_o = tick_n;
	tick_n = GetTickCount();
	Time += (tick_n - tick_o) / 1000.0;

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//��������� ���������
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//������ �����
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//������� ���  


	//





	s[0].UseShader();

	//�������� ���������� � ������.  ��� ���� - ���� ����� uniform ���������� �� �� �����. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//��� 2 - �������� �� ��������
	glUniform3fARB(location, light.pos.X(), light.pos.Y(),light.pos.Z());

	location = glGetUniformLocationARB(s[0].program, "Ia");
	glUniform3fARB(location, 0.2, 0.2, 0.2);

	location = glGetUniformLocationARB(s[0].program, "Id");
	glUniform3fARB(location, 1.0, 1.0, 1.0);

	location = glGetUniformLocationARB(s[0].program, "Is");
	glUniform3fARB(location, .7, .7, .7);


	location = glGetUniformLocationARB(s[0].program, "ma");
	glUniform3fARB(location, 0.2, 0.2, 0.1);

	location = glGetUniformLocationARB(s[0].program, "md");
	glUniform3fARB(location, 0.4, 0.65, 0.5);

	location = glGetUniformLocationARB(s[0].program, "ms");
	glUniform4fARB(location, 0.9, 0.8, 0.3, 25.6);

	location = glGetUniformLocationARB(s[0].program, "camera");
	glUniform3fARB(location, camera.pos.X(), camera.pos.Y(), camera.pos.Z());

	//������ ��������
	//objModel.DrawObj();


	Shader::DontUseShaders();
	
	//������, ��� ��������
	//glPushMatrix();
	//	glTranslated(-5,15,0);
	//	//glScaled(-1.0,1.0,1.0);
	//	objModel.DrawObj();
	//glPopMatrix();



	//��������
	double P1[] = { 0,0,4 }; //���� �����
	double P2[] = { 7,7,4 };
	double P3[] = { 8,10,5 };
	double P4[] = { 4,15,6 };

	/*s[1].UseShader();
	int l = glGetUniformLocationARB(s[1].program,"Ship_Base_Color"); 
	glUniform1iARB(l, 0); */    //��� ��� ����� �� ��������� �������� ������� �� GL_TEXTURE0
	
	angle1 += 5;
	if (rezh)
	{
		glPushMatrix();
		glTranslated(5 + x, 5 + y, 5 + z);

		if (a == 1)
		{

			glRotated(30, 0, 1, 0);

		}

		if (b == 1)
		{
			glRotated(-30, 0, 1, 0);
		}
		if (c == 1)
		{
			glRotated(-30, 1, 0, 0);
		}
		if (d == 1)
		{
			glRotated(30, 1, 0, 0);
		}
		glRotated(angle1, 0, 0, 1);
		monkeyTex.bindTexture();
		monkey.DrawObj();
		glPopMatrix();
	}
	else
		Bese2(P1, P2, P3, P4, Search_delta_time());

	//���������

	ploskTex.bindTexture();
	plosk.DrawObj();

	//���

	glPushMatrix();
	glTranslated(5,5,0);
	glRotated(180,0,0,1);
	glScaled(0.05, 0.05, 0.05);
	domTex.bindTexture();
	dom.DrawObj();
	glPopMatrix(); 

	glPushMatrix();
	glTranslated(-5, 0, 0);
	glRotated(180, 0, 0, 1);
	glScaled(0.05, 0.05, 0.05);
	dom2Tex.bindTexture();
	dom2.DrawObj();
	glPopMatrix();



	//������� ������

	glPushMatrix();

	
	glScaled(0.3, 0.3, 0.3);

	if (time1 <= 12)
	{

		glTranslated(7, 8, 3.5);
		glRotated(20,0,0,1);
		glRotated(20, 1, 0, 0);
		num = 1;
	}
	if (time1 <= 24 && time1>12 )
	{

		glTranslated(7, 8, 3.5);
		glRotated(20, 0, 0, 1);
		glRotated(20, 1, 0, 0);
		num = 0;
	}
	if (time1 <= 36 && time1>24)
	{

		glTranslated(7, 8, 3.5);
		glRotated(20, 0, 0, 1);
		glRotated(20, 1, 0, 0);
		num = 1;
	}
	if (time1 <= 42 && time1 > 36)
	{
		glTranslated(7, 7, 3.5);
		num = 0;
	}
	if (time1 <= 54 && time1 > 42)
	{

		glTranslated(7, 6, 3.5);
		glRotated(-20, 0, 0, 1);
		glRotated(-20, 1, 0, 0);
		num = 2;
	}
	if (time1 <= 66 && time1 > 54)
	{

		glTranslated(7, 6, 3.5);
		glRotated(-20, 0, 0, 1);
		glRotated(-20, 1, 0, 0);
		num = 0;
	}
	if (time1 <= 78 && time1 > 66)
	{

		glTranslated(7, 6, 3.5);
		glRotated(-20, 0, 0, 1);
		glRotated(-20, 1, 0, 0);
		num = 2;
	}
	if (time1 <= 84 && time1 > 78)
	{
		glTranslated(7, 7, 3.5);
		num = 0;
	}
	//glRotated(180, 0, 0, 1);
	if (num==1)
	{
		cow1Tex.bindTexture();
		cow1.DrawObj();
	}
	if (num == 2)
	{
		cow2Tex.bindTexture();
		cow2.DrawObj();
	}
	if (num == 0)
	{
		cowTex.bindTexture();
		cow.DrawObj();

	}
	

	
	glPopMatrix();

	time1++;
	if (time1 == 84)
	{
		time1 = 0;
	}
	

	
	
	

	
	Shader::DontUseShaders();
	
	
	
	
	
	
	
	
	//�����

	glMatrixMode(GL_PROJECTION);	
					
	glPushMatrix();   			    
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();			  
	glLoadIdentity();		  

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		 
	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 200 - 10);


	std::stringstream ss;
	ss << "T - ���/���� �������" << std::endl;
	ss << "L - ���/���� ���������" << std::endl;
	ss << "F - ���� �� ������" << std::endl;
	ss << "G - ������� ���� �� �����������" << std::endl;
	ss << "G+��� ������� ���� �� ���������" << std::endl;
	ss << "O - ������������ ������� �������� ���" << std::endl;
	ss << "W,A,S,D,Y,H - ���������� ���" << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	 
	glPopMatrix();
	
	
}   //����� ���� �������


bool gui_init = false;

//������ ���������, ��������� ����� �������� �������
void RenderGUI(OpenGL *ogl)
{
	
	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	

	


		
	Shader::DontUseShaders(); 



	
}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 100 - 10);
}

