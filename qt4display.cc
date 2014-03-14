/************* Interface to Qt4 graphics library *************
// Mac Clayton
// mckenzieclayton@gatech.edu
// Spring 2013
// Tested with Qt4.8.5 on OSX 10.9
*************************************************************/

#include <iostream>
#include <fstream>
#include "qt4display.h"
#include <QTimer>
#include <QImage>
#include <QRect>
#include <QPaintEvent>
#include <QPainter>

using namespace std;

int QDisplay::windowCount = 0;

// QApp methods
QApp::QApp(int argc, char** argv) 
    : QApplication(argc, argv)
{
  	// Connect application "lastwindowclosed" signal
  	connect(this, SIGNAL(lastWindowClosed()), SLOT(quit()));
}

void QApp::Run()
{
  	exec();
}

// Constructors
QDisplay::QDisplay(QApplication& a)
    : ready(true), closed(false), qTimer(0), image(0), app(a),
    updateRate(0), colorTable(0), white(0), whitew(0), whiteh(0), 
    whited(0), msTime(0), paused(false)
{
	// Create Mutex:
	pthread_mutex_init(&updateMutex, 0);
    // Create and connect the timer
    qTimer = new QTimer();
    qTimer->setSingleShot(true);
    connect(qTimer, SIGNAL(timeout()), SLOT(timerDone()));
	
	saveTimer = new QTimer();
	connect(saveTimer, SIGNAL(timeout()), SLOT(timerDone()));

	userTimers = new QTimer[2];
	connect(&userTimers[0], SIGNAL(timeout()), SLOT(UserTimer0Done()));
	connect(&userTimers[1], SIGNAL(timeout()), SLOT(UserTimer1Done()));
	userReady = new bool[2];
    // Connect application "lastwindowclosed" signal
    connect(&app, SIGNAL(lastWindowClosed()), SLOT(mainClosed()));
}

QDisplay::~QDisplay()
{ // Destructor
	if (qTimer) qTimer->stop();
	delete qTimer;
	delete saveTimer;
	delete image;
  	delete white;
}

// Member functions

// Load an image from a file
bool QDisplay::Load(const char* fn)
{ 
    image = new QImage();
    if (!image->load(fn))
    { // load failed
      	cout << "Loading Image Failed" << endl;
        delete image;
        image = 0;
        return false;
    }
    resize(image->width(), image->height());
    Update();
    windowCount++;
    return true;
}

bool QDisplay::Save(const  char* fn)
{
    if (!image) return false; // no image to save
    fstream fs(fn);
    if (!fs) return false;
    fs << "P5" << endl;
    fs << "# Created by Mac Clayton" << endl;
    fs << "# GENERATOR: QDisplay::Save" << endl;
    fs << image->width() << " " << image->height() << endl;
    fs << image->numColors() << endl;
    unsigned char* b = image->bits();
    for (int y = 0; y < image->height(); ++y)
    {
        for (int x = 0; x < image->width(); ++x)
        {
          	fs << b[y*image->width() + x];
        }
    }
  	fs.close();
  	return true;
}

// Create a blank (all white) grayscale image
void QDisplay::BlankImage(int w, int h, int d)
{
	int colorCount;

	// If Image is 8-bit:
  	if(d == 8)
  	{
    	colorCount = 256;
      	if (colorTable.empty())
      	{
          	for (int i = 0; i < 256; ++i)
          	{
              	colorTable.push_back(QColor(i, i, i).rgb());
          	}
      	}
      	if (whitew != w || whiteh != h)
      	{ // Existing white buffer wrong size
          	delete [] white;
          	white = new unsigned char[w * h];
          	memset(white, 0xff, w * h);
      	}  
    }
    // Otherwise:
    else
    {
      	colorCount = 0;
      	if(whitew!= w || whiteh != h || whited != d)
      	{
        	delete[] white;
        	white = (unsigned char*)new QRgb[w * h];
        	for(int i = 0; i < w * h; ++i)
        	{
          	((QRgb*)white)[i] = QColor(255, 255, 255).rgb();
        	}
      	}
    }

    // Update stored dimensions
  	whitew = w;
  	whiteh = h;
  	whited = d;

  	// Delete any existing image
    if (image)
    {
        delete image;
        image = 0;
    }
    if(d == 8)
    {
      	image = new QImage(white, w, h, QImage::Format_Indexed8);
      	image->setColorTable(colorTable);
    }
    else
    {
      	image = new QImage(white, w, h, QImage::Format_RGB32);
    }
    cout << "Depth of image:" << image->depth() << endl;
    cout << "Number of colors: " << colorTable.size() << endl;
    cout << "Width: " << w << " Height: " << h << endl;
    resize(image->width(), image->height());
    Update();
	windowCount++;
}

// Return image depth (bits per pixel), either 1, 8, or 32.
int  QDisplay::Depth()
{
    if (!image) return 0; 	// No image loaded
    int d = image->depth();
    if (d == 24) d = 32;	// If depth is 24, return 32
    return d;
}

// Return image width (pixels)
int  QDisplay::Width()
{ 
    if (!image) return 0;
    return image->width();
}

// Return image height (pixels)
int  QDisplay::Height()
{ 
    if (!image) return 0;
    return image->height();
}

// Get a pointer to the image data
unsigned char* QDisplay::ImageData()
{ 
    if(!image)
    {
    	cout << "No image loaded" << endl;
		return 0;
    }
    cout << "Image data has " << image->numBytes() << " bytes " << endl;
    return image->bits();
}

void  QDisplay::Update()
{
    if (closed) return;
    pthread_mutex_lock(&updateMutex);
    while(!ready && !closed) app.processEvents(); // Wait for 25fps elapsed 
    if (updateRate)
    {
        ready = false;
        qTimer->start(1000/updateRate);  // 40ms is 25 frames per secnod
    }
    update();
    pthread_mutex_unlock(&updateMutex);
}

void  QDisplay::Update(int x, int y, int w, int h, bool noPaint)
{
    if(closed) return;
    pthread_mutex_lock(&updateMutex);
    if(!noPaint)
    {
    	while(!ready && !closed) app.processEvents(); // Wait for 25fps elapsed 
    	if (updateRate)
    	{
        	ready = false;
        	qTimer->start(1000/updateRate);  // 40ms is 25 frames per secnod
    	}
    }

    if(image){
    	repaint(x, y, 1, 1);
    	update(x, y, w, h);
    }
    if(!noPaint) app.processEvents();
    pthread_mutex_unlock(&updateMutex);
}

void QDisplay::Update(const QRect& r, bool noPaint)
{
  	Update(r.x(), r.y(), r.width(), r.height(), noPaint);
}

bool QDisplay::IsClosed()
{
  	return closed;
}

void QDisplay::UpdateRate(int r)
{
  	updateRate = r;
}

void QDisplay::Run()
{
  	app.exec();
}

void QDisplay::Show(bool s)
{
  	if(s) show();
  	else hide();
}

void QDisplay::WaitUserTimer(int i, int ms)
{ // Wait for i'th timer, ms milliseconds
	return;
  
	userReady[i] = false;
	userTimers[i].start(ms);
	while(!userReady[i])
	{
		app.processEvents();
		//pthread_yield_np();
	}
}

void QDisplay::StartMSTimer()
{
	if (!msTimer)
	{
		msTimer = new QTimer();
		connect(msTimer, SIGNAL(timeout()), SLOT(MSTimerDone()));
		msTimer->start(10);
	}
}

void QDisplay::WaitForMS(int endMS)
{
	int endTime = msTime + endMS;
	StartMSTimer();
	while (msTime < endTime || paused)
	{
		//cout << "Waiting ms, now " << msTime << " endtime " << endTime << endl;
		app.processEvents();
		//pthread_yield_np();
	}
}




// Inherited from QMainWindow
void QDisplay::paintEvent(QPaintEvent* pe)
{
    if (image)
    {
        QRect r = pe->rect();
        QPainter painter(this);
        painter.drawImage(r.x(), r.y(), *image, r.x(), r.y(), r.width(), r.height());
    }
}

void QDisplay::mousePressEvent(QMouseEvent* e)
{
  	cout << "Mouse pressed, x " << e->x()
    	 << " y " << e->y() << endl;
}

// Slots
void QDisplay::timerDone()
{
    ready = true; // Enough time elapsed for another frame update
}

void QDisplay::UserTimer0Done()
{
	userReady[0] = true; // Enough time elapsed for another frame update
}

void QDisplay::UserTimer1Done()
{
	userReady[1] = true; // Enough time elapsed for another frame update
}

void QDisplay::MSTimerDone()
{
	msTime += 10;
	msTimer->start(10);
}

void QDisplay::SaveTimerDone()
{
	//
}

void QDisplay::mainClosed()
{
    closed = true; // Main window has closed
}

