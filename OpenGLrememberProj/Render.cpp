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


//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //массивчик для десяти шейдеров
Shader frac;
Shader cassini;

double x = 0;
double y = 0;
double z = 0;
int a, b, c, d = 0;
long int angle1 = 0;
long int time1 = 0;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
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



}  camera;   //создаем объект камеры


//класс недоделан!
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


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
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
			//линия от источника света до окружности
				glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
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

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света



//старые координаты мыши
int mouseX = 0, mouseY = 0;




float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//обработчик движения мыши
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
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


	
	//двигаем свет по плоскости, в точку где мышь
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

//обработчик вращения колеса  мыши
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
//обработчик нажатия кнопок клавиатуры
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

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{

	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	
	


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   загрузка текстуры из файла
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	frac.FshaderFileName = "shaders\\frac.frag"; //имя файла фрагментного шейдера
	frac.LoadShaderFromFile(); //загружаем шейдеры из файла
	frac.Compile(); //компилируем

	cassini.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	cassini.FshaderFileName = "shaders\\cassini.frag"; //имя файла фрагментного шейдера
	cassini.LoadShaderFromFile(); //загружаем шейдеры из файла
	cassini.Compile(); //компилируем
	

	s[0].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\light.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	s[1].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //имя файла фрагментного шейдера
	s[1].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[1].Compile(); //компилируем

	

	 //так как гит игнорит модели *obj_m файлы, так как они совпадают по расширению с объектными файлами, 
	 // создающимися во время компиляции, я переименовал модели в *obj_m_m
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
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "O - режимы движения НЛО" << std::endl;
	ss << "W,A,S,D,Y,H - управление НЛО" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "CTRL+S сохранить текущее состояние поверхности" << std::endl;
	ss << "CTRL+U загрузить поверхность из файла" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

	rec.setText(ss.str().c_str());*/

	
}

int num = 0;
double f(double p1, double p2, double p3, double t)
{
	return p1 * (1 - t) * (1 - t) * (1 - t) + 2 * p2 * t * (1 - t) + p3 * t * t; //посчитанная формула
}
double f(double p1, double p2, double p3, double p4, double t)
{
	return p1 * (1 - t) * (1 - t) * (1 - t) + 3 * p2 * t * (1 - t) * (1 - t) + 3 * p3 * t * t * (1 - t) + p4 * t * t * t; //посчитанная формула
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
		t_max += delta_time / 5; //t_max становится = 1 за 5 секунд
		if (t_max > 1)
		{
			t_max = 1; //после обнуляется
			flagReverse = !flagReverse;
		}
	}
	else
	{
		t_max -= delta_time / 5; //t_max становится = 1 за 5 секунд
		if (t_max < 0)
		{
			t_max = 0; //после обнуляется
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

	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//Прогать тут  


	//





	s[0].UseShader();

	//передача параметров в шейдер.  Шаг один - ищем адрес uniform переменной по ее имени. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//Шаг 2 - передаем ей значение
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

	//первый пистолет
	//objModel.DrawObj();


	Shader::DontUseShaders();
	
	//второй, без шейдеров
	//glPushMatrix();
	//	glTranslated(-5,15,0);
	//	//glScaled(-1.0,1.0,1.0);
	//	objModel.DrawObj();
	//glPopMatrix();



	//обезьяна
	double P1[] = { 0,0,4 }; //Наши точки
	double P2[] = { 7,7,4 };
	double P3[] = { 8,10,5 };
	double P4[] = { 4,15,6 };

	/*s[1].UseShader();
	int l = glGetUniformLocationARB(s[1].program,"Ship_Base_Color"); 
	glUniform1iARB(l, 0); */    //так как когда мы загружали текстуру грузили на GL_TEXTURE0
	
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

	//плоскость

	ploskTex.bindTexture();
	plosk.DrawObj();

	//дом

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



	//поворот коровы

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
	
	
	
	
	
	
	
	
	//текст

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
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертикали" << std::endl;
	ss << "O - переключение режимов движения НЛО" << std::endl;
	ss << "W,A,S,D,Y,H - управление НЛО" << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	 
	glPopMatrix();
	
	
}   //конец тела функции


bool gui_init = false;

//рисует интерфейс, вызывется после обычного рендера
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

