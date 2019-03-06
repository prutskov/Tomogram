﻿
// TomogramDlg.cpp: файл реализации
//

#include "stdafx.h"
#include "Tomogram.h"
#include "TomogramDlg.h"
#include "afxdialogex.h"
#include <numeric>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Диалоговое окно CTomogramDlg



CTomogramDlg::CTomogramDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TOMOGRAM_DIALOG, pParent)
	, _step_d(5)
	, _step_a(5)
	, _angle_max(180)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTomogramDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IMAGE, imageDrawer);
	DDX_Control(pDX, IDC_IMAGE2, drawerTomogram);
	DDX_Control(pDX, IDC_RESOLUTION, _resolutionText);
	DDX_Text(pDX, IDC_STEP_D, _step_d);
	DDX_Text(pDX, IDC_STEP_A, _step_a);
	DDX_Control(pDX, IDC_RESOLUTION2, _resolutionTomText);
	DDX_Text(pDX, IDC_MAX_ANGLE, _angle_max);
	DDX_Control(pDX, IDC_IMAGE3, _drawerRestored);
}

BEGIN_MESSAGE_MAP(CTomogramDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LOAD, &CTomogramDlg::OnBnClickedLoad)
	ON_BN_CLICKED(IDC_TOMOGRAM, &CTomogramDlg::OnBnClickedTomogram)
	ON_BN_CLICKED(IDC_RESTORE, &CTomogramDlg::OnBnClickedRestore)
END_MESSAGE_MAP()


// Обработчики сообщений CTomogramDlg

BOOL CTomogramDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Задает значок для этого диалогового окна.  Среда делает это автоматически,
	//  если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок

	// TODO: добавьте дополнительную инициализацию
	imageDrawer._image = &_image;
	drawerTomogram._image = &_imageTomogram;

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

// При добавлении кнопки свертывания в диалоговое окно нужно воспользоваться приведенным ниже кодом,
//  чтобы нарисовать значок.  Для приложений MFC, использующих модель документов или представлений,
//  это автоматически выполняется рабочей областью.

void CTomogramDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// Система вызывает эту функцию для получения отображения курсора при перемещении
//  свернутого окна.
HCURSOR CTomogramDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTomogramDlg::OnBnClickedLoad()
{
	LoadPicture();
	IncreaseSizeImage();
	imageDrawer._image = &_imageIncreased;
	imageDrawer.Invalidate();
}

void CTomogramDlg::LoadPicture()
{
	CFileDialog fd(true, NULL, NULL, OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
		OFN_LONGNAMES | OFN_PATHMUSTEXIST, _T("Image Files (*.bmp)|*.bmp|All Files (*.*)|*.*||"), NULL, 0, TRUE);

	if (fd.DoModal() != IDOK)
	{
		MessageBox(L"ERROR!!!", L"Error opening picture file.", MB_ICONERROR);
	};

	CString pathBMP = fd.GetPathName();
	Bitmap bmp(pathBMP);
	int width = bmp.GetWidth();
	int height = bmp.GetHeight();
	_image.clear();

	for (size_t i = 0; i < height; i++)
	{
		std::vector<float> bufPix;
		for (size_t j = 0; j < width; j++)
		{
			double value;
			Color color;
			bmp.GetPixel(j, height - i - 1, &color);
			value = 0.299*color.GetRed() + 0.587*color.GetGreen() + 0.114*color.GetBlue();
			bufPix.push_back(value);
		}
		_image.push_back(bufPix);
	}
}

void CTomogramDlg::RotateImage(double angle, const imageType & dataIn,
	imageType & dataOut, const std::vector<size_t> &indexes)
{
	const int size = dataIn.size();
	dataOut.clear();
	std::vector<float> row(size);
	dataOut.resize(indexes.size(), row);

	Matrix matrix;
	Point pointNew;
	const Point center(size / 2, size / 2);
	matrix.Translate(center.X, center.Y);
	matrix.Rotate(angle);

	Rect rect(0, 0, size, size);
	for (size_t y = 0; y < indexes.size(); y++)
	{
		for (int x = 0; x < size; x++)
		{
			pointNew.X = x - center.X;
			pointNew.Y = indexes[y] - center.Y;
			matrix.TransformPoints(&pointNew);
			if (!rect.Contains(pointNew)) continue;
			dataOut[y][x] = dataIn[pointNew.Y][pointNew.X];
		}
	}
}

void CTomogramDlg::RotateFullImage(double angle, const imageType & dataIn,	imageType & dataOut)
{
	const int size = dataIn.size();
	dataOut.clear();
	std::vector<float> row(size);
	dataOut.resize(size, row);

	Matrix matrix;
	Point pointNew;
	const Point center(size / 2, size / 2);
	matrix.Translate(center.X, center.Y);
	matrix.Rotate(angle);

	Rect rect(0, 0, size, size);
	for (size_t y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			pointNew.X = x - center.X;
			pointNew.Y = y - center.Y;
			matrix.TransformPoints(&pointNew);
			if (!rect.Contains(pointNew)) continue;
			dataOut[y][x] = dataIn[pointNew.Y][pointNew.X];
		}
	}
}


void CTomogramDlg::IncreaseSizeImage()
{
	const int sizeNew = (int)sqrt(_image.size()*_image.size() + _image[0].size()*_image[0].size());
	const int widthOld = _image[0].size(),
		heightOld = _image.size();
	const int sizeNewHalf = sizeNew / 2;

	_imageIncreased.clear();
	std::vector<float> row(sizeNew);
	_imageIncreased.resize(sizeNew, row);

#pragma omp parallel for
	for (int y = 0; y < heightOld; y++)
	{
		for (int x = 0; x < widthOld; x++)
		{
			_imageIncreased[y + sizeNewHalf - heightOld / 2][x + sizeNewHalf - widthOld / 2] = _image[y][x];
		}
	}

	int widthText = _imageIncreased[0].size(),
		heightText = _imageIncreased.size();
	CString str, arg1, arg2;
	str = "Resolution: ";
	arg1.Format(_T("%d"), widthText);
	arg2.Format(_T("%d"), heightText);
	str += arg1 + "x" + arg2;
	_resolutionText.SetWindowTextW(str);
}



std::vector<float> CTomogramDlg::CreateTomogramRow(double angle, const std::vector<size_t> &indexes)
{
	std::vector<float> outRow(indexes.size());
	imageType rotatedCols;

	//Rotate columns by indexes
	RotateImage(angle, _imageIncreased, rotatedCols, indexes);

	//Get point of Radon converting
	for (size_t i = 0; i < indexes.size(); i++)
	{
		outRow[i] = std::accumulate(rotatedCols[i].begin(), rotatedCols[i].end(), 0);
	}

	return outRow;
}


void CTomogramDlg::OnBnClickedTomogram()
{
	UpdateData(TRUE);
	const size_t step = _step_d;
	const double angleStep = _step_a;
	std::vector<size_t> indexes;
	_imageTomogram.clear();

	//Create indexes of columns for radon converting
	for (size_t i = 0; i < _imageIncreased.size(); i+=step)
	{
		indexes.push_back(i);
	}

	for (double angle = 0; angle < _angle_max; angle += angleStep)
	{
		auto row = CreateTomogramRow(angle, indexes);
		_imageTomogram.push_back(row);
	}
	
	NormalizeAmplitude(_imageTomogram);
	drawerTomogram.Invalidate();
	int widthText = _imageTomogram[0].size(),
		heightText = _imageTomogram.size();
	CString str, arg1, arg2;
	str = "Resolution: ";
	arg1.Format(_T("%d"), widthText);
	arg2.Format(_T("%d"), heightText);
	str += arg1 + "x" + arg2;
	str += " (n_projection x n_angle)";
	_resolutionTomText.SetWindowTextW(str);
}

void CTomogramDlg::NormalizeAmplitude(imageType &data)
{
	float max = 0;
	for (size_t i = 0; i < data.size(); i++)
	{
		float local_max = *std::max_element(data[i].begin(), data[i].end());
		max = local_max > max ? local_max : max;
	}

#pragma omp parallel for
	for (int i = 0; i < data.size(); i++)
	{
		for (int j = 0; j < data[i].size(); j++)
		{
			data[i][j] = data[i][j] / max * 255.0;
		}
	}
	
}

void CTomogramDlg::BackProjection(imageType & dataOut)
{
	const double angleStep = _step_a;
	const size_t step = _step_d;

	const int size = _imageIncreased.size();
	
	dataOut.clear();
	std::vector<float> row(size);
	dataOut.resize(size, row);

	std::vector<size_t> indexes;
	imageType dataIn;
	dataIn.resize(size, row);

	for (size_t i = 0; i < _imageIncreased.size(); i += step)
	{
		indexes.push_back(i);
	}

	size_t numProjection = 0;
	for (double angle = 0; angle < _angle_max; angle += angleStep)
	{
		RotateFullImage(angle, dataIn, dataOut);
		
		//
#pragma omp parallel for
		for (int row = 0; row < indexes.size(); row++)
		{
			double value = _imageTomogram[numProjection][row] / indexes.size();
			for (int col = 0; col < dataOut[0].size(); col++)
			{
				dataOut[indexes[row]][col] += value;
			}
		}

		RotateFullImage(-angle, dataOut, dataIn);
		numProjection++;
	}

	//std::swap(dataIn, dataOut);
}


void CTomogramDlg::OnBnClickedRestore()
{
	FourierTransform(_imageTomogram, _imageRestored);
	BackProjection(_imageRestored);
	NormalizeAmplitude(_imageRestored);
	_drawerRestored._image = &_imageRestored;
	_drawerRestored.RedrawWindow();
	//FourierTransform(_imageTomogram, _imageRestored);
}

void CTomogramDlg::FourierTransform(imageType & dataIn, imageType & dataSpectre)
{
	const int nRows = dataIn.size();
	const int nCols = dataIn[0].size();
	for (int rowIdx = 0; rowIdx < nRows; rowIdx++)
	{		
		/*std::vector<complex<float>> row(nCols, 0);
		for (int colIdx = 0; colIdx < nCols; colIdx++)
		{
			row[colIdx]._Val[0] = dataIn[rowIdx][colIdx];
		}*/
		fftw_complex* rowIn = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nCols);
		fftw_complex* rowOut = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nCols);
		for (int colIdx = 0; colIdx < nCols; colIdx++)
		{
			rowIn[colIdx][0] = dataIn[rowIdx][colIdx];
			rowIn[colIdx][1] = 0;
		}

		/*fftw_plan plan = fftw_plan_dft_1d(row.size(), (fftw_complex*)&row[0],
							(fftw_complex*)&row[0], FFTW_FORWARD, FFTW_ESTIMATE);*/

		fftw_plan plan = fftw_plan_dft_1d(nCols, rowIn, rowOut, FFTW_FORWARD, FFTW_ESTIMATE);

		fftw_execute(plan);
		fftw_destroy_plan(plan);

		for (int colIdx = 0; colIdx < nCols; colIdx++)
		{
			dataIn[rowIdx][colIdx] = sqrt(rowOut[colIdx][0]* rowOut[colIdx][0] +
				rowOut[colIdx][1] * rowOut[colIdx][1]);
		}

		fftw_free(rowIn);
		fftw_free(rowOut);

	}
	int k = 0;
}
