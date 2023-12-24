#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

#include "math.h"

bool textureMode = true;
bool lightMode = true;

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
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
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
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


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
		glDisable(GL_LIGHTING);

		
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

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
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

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

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

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

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
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
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

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

void normal(double A[], double B[], double C[], double k)
{
	double BC[] = { A[0] - B[0], A[1] - B[1], A[2] - B[2] };
	double BA[] = { C[0] - B[0], C[1] - B[1], C[2] - B[2] };
	double N[] = { (k * (BA[1] * BC[2]) - (BC[1] * BA[2])), k * ((BA[0] * BC[2] * -1) + (BC[0] * BA[2])), k * ((BA[0] * BC[1]) - (BC[0] * BA[1])) };
	double length = sqrt(pow(N[0], 2) + pow(N[1], 2) + pow(N[2], 2));

	N[0] /= length;
	N[1] /= length;
	N[2] /= length;

	glNormal3d(N[0], N[1], N[2]);
}

void normal_walls(double A[], double B[], double C[])
{
	double BC[] = { A[0] - B[0], A[1] - B[1], A[2] - B[2] };
	double BA[] = { C[0] - B[0], C[1] - B[1], C[2] - B[2] };
	double N[] = { -1 * ((BA[1] * BC[2]) - (BC[1] * BA[2])), ((BA[0] * BC[2] * -1) + (BC[0] * BA[2])), ((BA[0] * BC[1]) - (BC[0] * BA[1])) };
	double length = sqrt(pow(N[0], 2) + pow(N[1], 2) + pow(N[2], 2));

	N[0] /= length;
	N[1] /= length;
	N[2] /= length;

	glNormal3d(N[0], N[1], N[2]);
}



void bottom_edge()
{
	double A[] = { 0, 1, 0 };
	double B[] = { -2, 0, 0 };
	double C[] = { 0, -1, 0 };
	
	glBegin(GL_TRIANGLES);
	normal(A, B, C, -1);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glEnd();
	
	double A1[] = { 0, 1, 0 };
	double B1[] = { -4, 4, 0 };
	double C1[] = { -2, 0, 0 };
	
	glBegin(GL_TRIANGLES);
	normal(A1, B1, C1, -1);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glEnd();

	double A2[] = { 0, -1, 0 };
	double B2[] = { -2, 0, 0 };
	double C2[] = { -1, -5, 0 };
	
	glBegin(GL_TRIANGLES);
	normal(A2, B2, C2, -1);
	glVertex3dv(A2);
	glVertex3dv(B2);
	glVertex3dv(C2);
	glEnd();

	double A3[] = { -2, 0, 0 };
	double B3[] = { -1, -5, 0 };
	double C3[] = { -6, -4, 0 };
	
	glBegin(GL_TRIANGLES);
	normal(A3, B3, C3, 1);
	glVertex3dv(A3);
	glVertex3dv(B3);
	glVertex3dv(C3);
	glEnd();

	double A4[] = { 0, -1, 0 };
	double B4[] = { 0, 1, 0 };
	double C4[] = { 6, 6, 0 };
	
	glBegin(GL_TRIANGLES);
	normal(A4, B4, C4, 1);
	glVertex3dv(A4);
	glVertex3dv(B4);
	glVertex3dv(C4);
	glEnd();

	double A5[] = { 0, -1, 0 };
	double B5[] = { 7, -3, 0 };
	double C5[] = { 6, 6, 0 };
	
	glBegin(GL_TRIANGLES);
	normal(A5, B5, C5, -1);
	glVertex3dv(A5);
	glVertex3dv(B5);
	glVertex3dv(C5);
	glEnd();
}

void upper_edge()
{
	double A[] = { 0, 1, 10 };
	double B[] = { -2, 0, 10 };
	double C[] = { 0, -1, 10 };

	glBegin(GL_TRIANGLES);
	normal(A, B, C, 1);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glEnd();



	double A1[] = { 0, 1, 10 };
	double B1[] = { -4, 4, 10 };
	double C1[] = { -2, 0, 10 };
	
	glBegin(GL_TRIANGLES);
	normal(A1, B1, C1, 1);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glEnd();

	double A2[] = { 0, -1, 10 };
	double B2[] = { -2, 0, 10 };
	double C2[] = { -1, -5, 10 };
	
	glBegin(GL_TRIANGLES);
	normal(A2, B2, C2, 1);
	glVertex3dv(A2);
	glVertex3dv(B2);
	glVertex3dv(C2);
	glEnd();

	double A3[] = { -2, 0, 10 };
	double B3[] = { -1, -5, 10 };
	double C3[] = { -6, -4, 10 };
	
	glBegin(GL_TRIANGLES);
	normal(A3, B3, C3, -1);
	glVertex3dv(A3);
	glVertex3dv(B3);
	glVertex3dv(C3);
	glEnd();

	double A4[] = { 0, -1, 10 };
	double B4[] = { 0, 1, 10 };
	double C4[] = { 6, 6, 10 };
	
	glBegin(GL_TRIANGLES);
	normal(A4, B4, C4, -1);
	glVertex3dv(A4);
	glVertex3dv(B4);
	glVertex3dv(C4);
	glEnd();

	double A5[] = { 0, -1, 10 };
	double B5[] = { 7, -3, 10 };
	double C5[] = { 6, 6, 10 };
	
	glBegin(GL_TRIANGLES);
	normal(A5, B5, C5, 1);
	glVertex3dv(A5);
	glVertex3dv(B5);
	glVertex3dv(C5);
	glEnd();
}

void walls()
{
	double A[] = { 0, 1, 0 };
	double B[] = { -4, 4, 0 };
	double C[] = { -4, 4, 10 };
	double D[] = { 0, 1, 10 };
	
	glBegin(GL_QUADS);
	normal(A, B, C, 1);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);
	glEnd();

	double A1[] = { -2, 0, 0 };
	double B1[] = { -4, 4, 0 };
	double C1[] = { -4, 4, 10 };
	double D1[] = { -2, 0, 10 };
	
	glBegin(GL_QUADS);
	normal(B1, C1, D1, -1);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glEnd();

	double A2[] = { -2, 0, 0 };
	double B2[] = { -6, -4, 0 };
	double C2[] = { -6, -4, 10 };
	double D2[] = { -2, 0, 10 };
	
	glBegin(GL_QUADS);
	normal(A2, B2, C2, 1);
	glVertex3dv(A2);
	glVertex3dv(B2);
	glVertex3dv(C2);
	glVertex3dv(D2);
	glEnd();

	double A3[] = { 0, -1, 0 };
	double B3[] = { -1, -5, 0 };
	double C3[] = { -1, -5, 10 };
	double D3[] = { 0, -1, 10 };
	
	glBegin(GL_QUADS);
	normal(B3, C3, D3, -1);
	glVertex3dv(A3);
	glVertex3dv(B3);
	glVertex3dv(C3);
	glVertex3dv(D3);
	glEnd();

	double A4[] = { -1, -5, 0 };
	double B4[] = { -6, -4, 0 };
	double C4[] = { -6, -4, 10 };
	double D4[] = { -1, -5, 10 };
	glBegin(GL_QUADS);
		glColor3d(0, 1, 0);
		glVertex3dv(A4);
		glVertex3dv(B4);
		glVertex3dv(C4);
		glVertex3dv(D4);
		glColor3d(0.5, 0.5, 0.5);
	glEnd();

	double A5[] = { 0, -1, 0 };
	double B5[] = { 7, -3, 0 };
	double C5[] = { 7, -3, 10 };
	double D5[] = { 0, -1, 10 };
	
	glBegin(GL_QUADS);
	normal(A5, B5, C5, 1);
	glVertex3dv(A5);
	glVertex3dv(B5);
	glVertex3dv(C5);
	glVertex3dv(D5);
	glEnd();

	double A6[] = { 7, -3, 0 };
	double B6[] = { 6, 6, 0 };
	double C6[] = { 6, 6, 10 };
	double D6[] = { 7, -3, 10 };
	
	glBegin(GL_QUADS);
	normal(A6, B6, C6, 1);
	glColor3d(1, 0, 1);
	glVertex3dv(A6);
	glVertex3dv(B6);
	glVertex3dv(C6);
	glVertex3dv(D6);
	glColor3d(0.5, 0.5, 0.5);
	glEnd();

	double A7[] = { 0, 1, 0 };
	double B7[] = { 6, 6, 0 };
	double C7[] = { 6, 6, 10 };
	double D7[] = { 0, 1, 10 };
	
	glBegin(GL_QUADS);
	normal(B7, C7, D7, -1);
	glVertex3dv(A7);
	glVertex3dv(B7);
	glVertex3dv(C7);
	glVertex3dv(D7);
	glEnd();
}

void tre()
{

	glBegin(GL_LINES);
	glVertex3d(-3.5, -4.5, 0);
	glVertex3d(-3.5, -4.5, 10);
	glEnd();

	double A1[] = { -6, -4, 10 };
	double B1[] = { -6, -5, 10 };
	double C1[] = { -3.5, -4.5, 10 };
	
	glBegin(GL_TRIANGLES);
	normal(A1, B1, C1, 1);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glEnd();

	double A2[] = { -6, -4, 0 };
	double B2[] = { -6, -5, 0 };
	double C2[] = { -3.5, -4.5, 0 };
	
	glBegin(GL_TRIANGLES);
	normal(A2, B2, C2, 1);
	glVertex3dv(A2);
	glVertex3dv(B2);
	glVertex3dv(C2);
	glEnd();

	double A3[] = { -3.5, -4.5, 0 };
	double B3[] = { -6, -5, 0 };
	double C3[] = { -6, -5, 10 };
	double D3[] = { -3.5, -4.5, 10 };
	
	glBegin(GL_QUADS);
	normal(A3, B3, C3, 1);
	glVertex3dv(A3);
	glVertex3dv(B3);
	glVertex3dv(C3);
	glVertex3dv(D3);
	glEnd();

	double A4[] = { -6, -4, 0 };
	double B4[] = { -6, -5, 0 };
	double C4[] = { -6, -5, 10 };
	double D4[] = { -6, -4, 10 };
	
	glBegin(GL_QUADS);
	normal(A4, B4, C4, 1);
	glVertex3dv(A4);
	glVertex3dv(B4);
	glVertex3dv(C4);
	glVertex3dv(D4);
	glEnd();
}

void convexity()
{
	glColor3d(0, 1, 0);
	for (int i = 169; i < 346; i += 2)
	{
		double x = -3.5 + 2.54915 * cos(i * (3.14 / 180.0));
		double y = -4.5 + 2.54915 * sin(i * (3.14 / 180.0));
		double x1 = -3.5 + 2.54915 * cos((i + 5) * (3.14 / 180.0));
		double y1 = -4.5 + 2.54915 * sin((i + 5) * (3.14 / 180.0));
		glBegin(GL_TRIANGLES);
		glVertex3d(-3.5, -4.5, 0);
		glVertex3d(x, y, 0);
		glVertex3d(x1, y1, 0);
		glEnd();

		glBegin(GL_TRIANGLES);
		glVertex3d(-3.5, -4.5, 10);
		glVertex3d(x, y, 10);
		glVertex3d(x1, y1, 10);
		glEnd();

		double A[] = { x, y, 0 };
		double B[] = { x1, y1, 0 };
		double C[] = { x1, y1, 10 };
		double D[] = { x, y, 10 };
		glBegin(GL_QUADS);
		glVertex3dv(A);
		glVertex3dv(B);
		glVertex3dv(C);
		glVertex3dv(D);
		glEnd();
	}
}








void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

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
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  
	glColor3d(0.5, 0.5, 0.5);
	upper_edge();
	bottom_edge();
	walls();
	convexity();

	////Начало рисования квадратика станкина
	//double A[2] = { -4, -4 };
	//double B[2] = { 4, -4 };
	//double C[2] = { 4, 4 };
	//double D[2] = { -4, 4 };

	//glBindTexture(GL_TEXTURE_2D, texId);

	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_QUADS);

	//glNormal3d(0, 0, 1);
	//glTexCoord2d(0, 0);
	//glVertex2dv(A);
	//glTexCoord2d(1, 0);
	//glVertex2dv(B);
	//glTexCoord2d(1, 1);
	//glVertex2dv(C);
	//glTexCoord2d(0, 1);
	//glVertex2dv(D);

	//glEnd();
	////конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}