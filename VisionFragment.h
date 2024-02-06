#ifndef VISIONFRAGMENT_H
#define VISIONFRAGMENT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <algorithm>

using namespace cv;

//Projenin özü olan Vision Fragment sınıfı.
//Bu sınıfın mantığı, görüntü işlemede sadece çiğ görüntü datası ile değil, birbiri ile iletişimde olan / olabilen
//görüntü datası oluşturmak. 
//Her Vision fragment objesi, kendi içinde işlemler yapabilir ve komşusu olan diğer Vision Fragment'lere
//onların adreslerini belirten bir pointer ile ulaşabilir (pointer vektörü oluşturulduktan sonra veriliyor).
class VisionFragment
{
private:
	int id;
	int suspectId = -1;
	int colorId = 0;
	glm::ivec2 center;
	int dimensions;
	cv::Mat image;
	cv::Scalar areaColor;
	std::vector<VisionFragment*> neighbours;
	std::vector<VisionFragment*>* suspectsRef;
	bool changedColor;
	vector<VisionFragment*>* counterRef;
	vector<vector<int>>* mapRef;
	vector<Scalar>* debugColors;
	//Fragment'in tekabül ettiği alandaki ortalama rengi hesaplayıp buradaki değişkene atamamızı sağlayan yardımcı fonksiyon.
	cv::Scalar getAvgColor()
	{
		cv::Rect roiRect(center.x - (dimensions / 2), center.y - (dimensions / 2), dimensions, dimensions);
		cv::Mat roi = image(roiRect);
		return cv::mean(roi);
	}
public:
	//Constructor
	VisionFragment(int id, glm::ivec2 center, int dimension, cv::Mat image, vector<VisionFragment*>* susRef, vector<vector<int>>* map, vector<Scalar>* debugVal)
	{
		VisionFragment::id = id;
		VisionFragment::center = center;
		VisionFragment::dimensions = dimension;
		VisionFragment::image = image;
		areaColor = getAvgColor();
		changedColor = false;
		suspectsRef = susRef;
		mapRef = map;
		debugColors = debugVal;
	}

	//Yinelemeli fonksiyon
	//Sadece benzer renkte olan Fragment'lar veya Controller tarafından birbirleri üzerinde çağrılabilir.
	//Eğer komşulardan bir tanesi farklı bir renkte ise, Fragment Controller'daki suspects vektörüne o komşuyu ekliyoruz
	//daha sonra suspects vektörü tamamen boşalana kadar bu fonksiyonu çağrıyoruz, çağırdığımızı vektörden çıkartıyoruz.
	//*YAPILACAK*
	//Henüz yapılacak birşey yok.
	void startObject(int color)
	{
		if (suspectId != -1)
		{
			auto it = std::find(suspectsRef->begin(), suspectsRef->end(), this);
			int index = std::distance(suspectsRef->begin(), it);
			suspectsRef->erase(suspectsRef->begin() + index);
		}

		//algortimanın bu fonksiyon üzerinden geçtiğini teyit et.
		changedColor = true;
		colorId = color;
		if (mapRef->size() < colorId)
		{
			vector<int> temp;
			mapRef->push_back(temp);
			mapRef->at(colorId - 1).push_back(center.x); //Minimum X
			mapRef->at(colorId - 1).push_back(center.x); //Maximum X
			mapRef->at(colorId - 1).push_back(center.y); //Minimum Y
			mapRef->at(colorId - 1).push_back(center.y); //Maximum Y
			mapRef->at(colorId - 1).push_back(1); //bu index'deki fragment sayısı
		}
		else
		{
			mapRef->at(colorId - 1).at(4)++;
			compareFragmentPositions(&mapRef->at(colorId - 1));
		}
		//debugDraw3();
		debugDraw2(to_string(colorId), 0.1, 1);
		imshow("process", image);
		waitKey(1);
		
		//komşuları kontrol et
		for (int i = 0; i < neighbours.size(); i++)
		{
			if (!neighbours[i]->getChangedColor()) //algoritmanın daha önceden geçtiği komuşuyu atla.
			{
				if (compareHSVColor(areaColor, neighbours[i]->getColor(), 1.5)) //eğer benzer renkte ise komşu üzerinde yineleme yap.
				{
					neighbours[i]->startObject(colorId);
				}
				else //değilse komşuyu suspects vektörüne ekle.
				{
					if (neighbours[i]->getSuspectId() == -1) //komşunun daha önce eklenmediğinden emin ol.
					{
						suspectsRef->push_back(neighbours[i]);
						neighbours[i]->setSuspecId(suspectsRef->size() - 1);
					}	
				}
			}
		}
	}

	//BoundingBox çizimi için kullanacağımız, bir renk kümesi içindeki en sağ, en sol, en alt ve en üst
	//noktaları belirlemeyi sağlayan yardımcı fonksiyon.
	void compareFragmentPositions(vector<int>* toApply)
	{
		if (center.x < toApply->at(0))
		{
			toApply->at(0) = center.x;
		}
		if (center.x > toApply->at(1))
		{
			toApply->at(1) = center.x;
		}
		if (center.y < toApply->at(2))
		{
			toApply->at(2) = center.y;
		}
		if (center.y > toApply->at(3))
		{
			toApply->at(3) = center.y;
		}
	}

	// Renk karşılaştırması ve HSV renkler falan şuan pek bilgim olmadığı, 
	//daha doğrusu bilgim olduğu ama yeterli olduğunu düşünmediğim konular.
	//Bundan dolayı bu fonksiyonu ChatGPT'den aldım, ama ilk fırsatta revize edeceğim.
	bool compareHSVColor(const Scalar& color1, const Scalar& color2, double threshold = 30) {
		// Convert the colors to HSV
		Mat hsvColor1, hsvColor2;
		cvtColor(Mat(1, 1, CV_8UC3, color1), hsvColor1, COLOR_BGR2HSV);
		cvtColor(Mat(1, 1, CV_8UC3, color2), hsvColor2, COLOR_BGR2HSV);

		Vec3b hsv1 = hsvColor1.at<Vec3b>(Point(0, 0)); // get HSV values for the pixel. Hue is the 1st in the vector
		Vec3b hsv2 = hsvColor2.at<Vec3b>(Point(0, 0));

		// Sum the differences in each channel
		double d1 = abs(hsv1(0) - hsv2(0));  // absolute distance
		double d = min(d1, 180 - d1); // correct distance if cross 0deg . full circle at 180 because of Hue scaling

		// Check if the total difference is below the threshold
		return d < threshold;
	}

	// *** GET / SET ***
	void setNeigbours(std::vector<VisionFragment*> val)
	{
		neighbours = val;
	}

	bool getChangedColor()
	{
		return changedColor;
	}

	cv::Scalar getColor()
	{
		return areaColor;
	}

	glm::vec2 getCenter()
	{
		return center;
	}

	void setChangedColor(bool val)
	{
		changedColor = val;
	}

	void setSuspecId(int id)
	{
		suspectId = id;
	}

	int getSuspectId()
	{
		return suspectId;
	}
	// *** GET / SET ***

	//Debug draw fonksiyonları...
	void debugDraw()
	{	
		cv::rectangle(image, cv::Point(center.x - (dimensions / 2), center.y - (dimensions / 2)), cv::Point(center.x + (dimensions / 2), center.y + (dimensions / 2)), areaColor, -1);
	}

	void debugDraw2(string text, double fontScale, double thickness)
	{
		cv::Size textSize = cv::getTextSize(text, FONT_HERSHEY_SIMPLEX, fontScale, thickness, nullptr);
		putText(image, to_string(colorId), Point(center.x - (textSize.width / 2), center.y + (textSize.height / 2)), FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 0), 1);
	}

	void debugDraw3()
	{
		int decider = colorId % debugColors->size();
		cv::rectangle(image, cv::Point(center.x - (dimensions / 2), center.y - (dimensions / 2)), cv::Point(center.x + (dimensions / 2), center.y + (dimensions / 2)), debugColors->at(decider), -1);
	}
};
#endif
