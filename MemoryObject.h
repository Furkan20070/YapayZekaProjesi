#ifndef MEMORYOBJECT_H
#define MEMORYOBJECT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <algorithm>
#include <fstream>
#include <string>
#include <cmath>

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
public:
	MemoryObject(int dimension, int boxWidth, int boxHeight, std::string file)
	{
		fragmentDimension = dimension;
		filedes = file;
		width = boxWidth;
		height = boxHeight;
		blackImage = Mat(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
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
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)5 / (float)3, 0.1f, 100.0f);
		//glm::mat4 projection = glm::ortho(-1.5f, 1.5f, -2.5f, 2.5f, 0.1f, 100.0f);
		glm::vec3 position(0.0f, 0.0f, -5.0f);
		glm::vec3 lookDir(0.0f, 0.0f, 1.0f);
		glm::vec3 up(0.0f, 1.0f, 0.0f);
		glm::mat4 view = glm::lookAt(position, position + lookDir, up);
		glm::mat4 model(1.0f);
		//model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		
		for (int i = 0; i < faces.size(); i++)
		{
			int testIndex = faces[i][faces[i].size() - 1] - 1;
			float dot = glm::dot(normals[testIndex], position);
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
					FindFragmentPos(x, y);
					if (j != 0)
					{
						cout << previous.x << " " << previous.y << endl;
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
					imshow("Black Image", blackImage);
					waitKey(0);
				}

			}
			else
			{
				std::cout << "pass!" << std::endl;
			}
		}

		/*
		for (int i = 0; i < vertices.size(); i++)
		{
			printVector3(vertices[i]);
			glm::vec4 vertex(vertices[i], 1.0f);
			glm::vec4 normalFinal = projection * view * model * vertex;
			std::cout << "vertex";
			std::cout << " " << normalFinal[0] << " " << normalFinal[1] << " " << normalFinal[2] << " " << normalFinal[3];
			std::cout << std::endl;
			glm::vec3 ndc(normalFinal[0] / normalFinal[3], normalFinal[1] / normalFinal[3], normalFinal[2] / normalFinal[3]);
			int x = (ndc[0] + 1) * width * 0.5f;
			int y = (ndc[1] + 1) * height * 0.5f;
			int centerX = static_cast<int>(x);
			std::cout << x << std::endl;
			std::cout << y << std::endl;

			//circle(blackImage, Point(250, 150), 5, Scalar(0, 0, 255), -1);
			Point center(x, y);
		    circle(blackImage, center, 5, Scalar(0, 0, 255), -1);
			imshow("Black Image", blackImage);
			waitKey(0);
		}*/
	}

	void FindFragmentPos(int x, int y)
	{
		int normalX = x / fragmentDimension;
		int normalY = y / fragmentDimension;
		int centerX = (fragmentDimension * (normalX + 1)) - (fragmentDimension / 2);
		int centerY = (fragmentDimension * (normalY + 1)) - (fragmentDimension / 2);
		rectangle(blackImage, Point(centerX - fragmentDimension / 2, centerY - fragmentDimension / 2), Point(centerX + fragmentDimension / 2, centerY + fragmentDimension / 2), Scalar(0, 0, 255), -1);
		
	}

	void MarkEdges(Point start, Point des)
	{
		float p = 1.0f / ((float)fragmentDimension * 2);
		for (int i = 0; i < fragmentDimension * 2; i++)
		{
			Point found = PointLerp(start, des, p * (i + 1));
			FindFragmentPos(found.x, found.y);
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
		int colAmount = width / fragmentDimension;
		int rowAmount = height / fragmentDimension;

		for (int i = 0; i < rowAmount; i++)
		{
			for (int j = 0; j < colAmount; j++)
			{
				int x = (fragmentDimension * (j + 1)) - (fragmentDimension / 2);
				int y = (fragmentDimension * (i + 1)) - (fragmentDimension / 2);
				rectangle(blackImage, Point(x - fragmentDimension / 2, y - fragmentDimension / 2), Point(x + fragmentDimension / 2, y + fragmentDimension / 2), Scalar(0, 0, 255), 1);
			}
		}
		imshow("Black Image", blackImage);
		waitKey(0);
	}
};
#endif
