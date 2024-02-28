#ifndef MEMORYOBJECT_H
#define MEMORYOBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "MemoryFragment.h"
#include <algorithm>
#include <fstream>
#include <string>
#include <cmath>
#include <conio.h>

class MemoryObject
{
private:
	int fragmentDimension;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<vector<int>> faces;
	std::string filedes;
	int height;
	int width;
	Mat blackImage;
	vector<vector<MemoryFragment>> fragments;
	int colAmount;
	int rowAmount;
	vector<int> fillDataY;
	vector<vector<int>> fillDataX;
	glm::mat4 projection;
	glm::vec3 position;
	glm::vec3 lookDir;
	glm::vec3 up;
	glm::mat4 view;
	glm::mat4 model;
public:
	MemoryObject(int dimension, int boxWidth, int boxHeight, std::string file)
	{
		fragmentDimension = dimension;
		filedes = file;
		width = boxWidth;
		height = boxHeight;
		blackImage = Mat(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
		colAmount = blackImage.cols / fragmentDimension;
		rowAmount = blackImage.rows / fragmentDimension;
		projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		position = glm::vec3(0.0f, 0.0f, -5.0f);
		lookDir = glm::vec3(0.0f, 0.0f, 1.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		view = glm::lookAt(position, position + lookDir, up);
		model = glm::mat4(1.0f);
	}

	void ImportModel()
	{
		std::ifstream inputFile;
		inputFile.open(filedes);
		if (!inputFile.is_open()) 
		{
			std::cerr << "Error opening the file!" << std::endl;
			return;
		}

		std::string line;
		while (std::getline(inputFile, line)) 
		{
			if (line[0] == 'v' && line[1] == ' ')
			{
				line.erase(0, 2);
				std::istringstream iss(line);
				glm::vec3 temp(1.0f);
				int helper = 0;
				float floatValue;
				while (iss >> floatValue) 
				{
					temp[helper] = floatValue;
					helper++;
				}
				vertices.push_back(temp);
			}

			if (line[0] == 'v' && line[1] == 'n')
			{
				line.erase(0, 3);
				std::istringstream iss(line);
				glm::vec3 temp(1.0f);
				int helper = 0;
				float floatValue;
				while (iss >> floatValue) 
				{
					temp[helper] = floatValue;
					helper++;
				}
				normals.push_back(temp);
			}

			if (line[0] == 'f' && line[1] == ' ')
			{
				line.erase(0, 2);
				std::istringstream iss(line);
				int checker = 0;
				bool finished = false;
				faces.push_back(vector<int>());
				int index = faces.size() - 1;
				while (!finished) 
				{
					char digitChar = line[checker];
					int digitInt = digitChar - '0';
					faces[index].push_back(digitInt);
					checker += 5;
					if (checker >= line.length())
						finished = true;
				}
				char digitChar = line[line.size() - 1];
				int digitInt = digitChar - '0';
				faces[index].push_back(digitInt);
			}
		}


		for (int i = 0; i < faces.size(); i++)
		{
			std::cout << "face";
			for (int j = 0; j < faces[i].size(); j++)
			{
				std::cout << " " << faces[i][j] << " ";
			}
			std::cout << std::endl;
		}

		inputFile.close();
	}

	void TestModel()
	{
		//blackImage = Mat(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
		for (int i = 0; i < faces.size(); i++)
		{
			int testIndex = faces[i][faces[i].size() - 1] - 1;
			glm::vec3 normalFinal = glm::mat3(glm::transpose(glm::inverse(model))) * normals[testIndex];
			float dot = glm::dot(normalFinal, position);
			Point first;
			Point previous;
			if (dot > 0)
			{
				for (int j = 0; j < faces[i].size() - 1; j++)
				{
					glm::vec4 vertex(vertices[faces[i][j] - 1], 1.0f);
					glm::vec4 vertexFinal = projection * view * model * vertex;
					glm::vec3 ndc(vertexFinal[0] / vertexFinal[3], vertexFinal[1] / vertexFinal[3], vertexFinal[2] / vertexFinal[3]);
					int x = (ndc[0] + 1) * width * 0.5f;
					int y = (ndc[1] + 1) * height * 0.5f;
					Point center(x, y);
					FindFragmentPos(x, y, true);
					if (j != 0)
					{
						MarkEdges(previous, center);
						if (j == faces[i].size() - 2)
						{
							MarkEdges(center, first);
						}
					}
					else
					{
						first = center;
					}
					previous = center;
				}

			}
		}
		CompleteSurface();
		imshow("Black Image", blackImage);
		int key = waitKey(5000);
		switch (key)
		{
			case 119:
				rotateModel(-1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				break;
			case 115:
				rotateModel(1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
				break;
			case 100:
				rotateModel(1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			case 97:
				rotateModel(-1.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				break;
			default:
				break;
		}
		//cout << key << endl;
		fillDataY.clear();
		fillDataX.clear();
		for (int i = 0; i < rowAmount; i++)
		{
			for (int j = 0; j < colAmount; j++)
			{
				fragments[i][j].clearMark();
			}
		}
	}

	void FindFragmentPos(int x, int y, bool isEdge)
	{
		int normalX = x / fragmentDimension;
		int normalY = y / fragmentDimension;
		if (normalX < 0 || normalX > colAmount - 1 || normalY < 0 || normalY > rowAmount - 1)
		{
			return;
		}
		fragments[normalY][normalX].Mark(isEdge);
		auto result = std::find(fillDataY.begin(), fillDataY.end(), normalY);
		if (result == fillDataY.end())
		{
			fillDataY.push_back(normalY);
			vector<int> temp;
			temp.push_back(normalX);
			temp.push_back(normalX);
			fillDataX.push_back(temp);
		}
		else
		{
			int index = result - fillDataY.begin();
			if (normalX < fillDataX[index][0])
			{
				fillDataX[index][0] = normalX;
			}
			if (normalX > fillDataX[index][1])
			{
				fillDataX[index][1] = normalX;
			}
		}
	}

	void CompleteSurface()
	{
		for (int i = 0; i < fillDataY.size(); i++)
		{
			for (int j = fillDataX[i][0]; j < fillDataX[i][1] + 1; j++)
			{
				fragments[fillDataY[i]][j].Mark(fragments[fillDataY[i]][j].GetEdge());
			}
		}
	}

	void MarkEdges(Point start, Point des)
	{
		float p = 1.0f / ((float)fragmentDimension * 2);
		for (int i = 0; i < fragmentDimension * 2; i++)
		{
			Point found = PointLerp(start, des, p * (i + 1));
			FindFragmentPos(found.x, found.y, true);
		}
	}

	Point PointLerp(Point A, Point B, float amount)
	{
		int x = A.x * (1.0f - amount) + B.x * (amount);
		int y = A.y * (1.0f - amount) + B.y * (amount);
		return Point(x, y);
	}

	void printVector3(glm::vec3 toPrint)
	{
		std::cout << " " << toPrint[0] << " " << toPrint[1] << " " << toPrint[2] << " " << std::endl;
	}

	void debugTest()
	{

		for (int i = 0; i < rowAmount; i++)
		{
			vector<MemoryFragment> rowTemp;
			for (int j = 0; j < colAmount; j++)
			{
				int x = (fragmentDimension * (j + 1)) - (fragmentDimension / 2);
				int y = (fragmentDimension * (i + 1)) - (fragmentDimension / 2);
				MemoryFragment temp(j, i, Point(x, y), &blackImage, fragmentDimension);
				rowTemp.push_back(temp);
			}
			fragments.push_back(rowTemp);
		}
	}

	void rotateModel(float amount, glm::vec3 dir)
	{
		model = glm::rotate(model, glm::radians(amount), dir);
	}

	void debugControl()
	{
		int key;

		while (true) 
		{
			TestModel();
		}
	}

};
#endif
