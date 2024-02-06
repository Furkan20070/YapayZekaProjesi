#ifndef FRAGMENTCONTROLLER_H
#define FRAGMENTCONTROLLER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "VisionFragment.h"

//Vision Fragment objelerini yaptığımız ve genel olarak kontrol ettiğimiz sınıf. Genel olarak toplu objeleri içeren 
//fonksiyon ve hesaplamalar burada olacak.
class FragmentController
{
	std::vector<VisionFragment> fragments;
public:
    vector<VisionFragment*> suspects; //Bu, recursive algoritmada benzer olmayan fragment'lerin adreslerinin saklandığı vektör.
    vector<vector<int>> intensityMap; //Bulduğumuz objeler, kenarlar ve renk kümelerini matematiksel olarak ifade ettiğimiz değişken.
    vector<Scalar> debugColors; //debug
    int divider = 10; //Vision Fragment objelerinin piksel olarak büyüklüğü (hepsi kare).
    FragmentController(int divider)
    {
        this->divider = divider;
    }

    //Input olarak bir resim alıyoruz. Aldğımız resim üzerinde constructor'da
    //aldığımız divider ile orantılı olarak vision fragment objeleri oluşturuyoruz.
    //YAPILACAK
    //Fonksiyonu muhtemelen bölmem gerekiyor (oluştuma dışında ekstra birkaç olay daha var fonksiyon içinde).
    void createFragments(cv::Mat image)
    {
        //Tamamen Debug amaçlı kod parçası, fonksiyonun genel amacıyla alakası yok
        debugColors.push_back(Scalar(255, 204, 204));
        debugColors.push_back(Scalar(255, 204, 153));
        debugColors.push_back(Scalar(255, 255, 102));
        debugColors.push_back(Scalar(153, 255, 51));
        debugColors.push_back(Scalar(0, 255, 0));
        debugColors.push_back(Scalar(0, 204, 102));
        debugColors.push_back(Scalar(0, 153, 153));
        //debugColors.push_back(Scalar(0, 51, 102));
        //debugColors.push_back(Scalar(0, 0, 51));
        debugColors.push_back(Scalar(0, 0, 255));

        //Input olarak aldığımız resmin boyutlarını vision fragment'in piksellerine böl ve sayı elde et.
        int rowAmount = image.rows / divider;
        int colAmount = image.cols / divider;

        //Fragment'leri oluştur ve sakla.
        std::vector<VisionFragment> fragments;
        for (int i = 0; i < rowAmount; i++)
        {
            for (int j = 0; j < colAmount; j++)
            {
                //Fragmentlerin id'si ve resim üzerindeki pozisyonunu hesaplıyoruz.
                int id = (i * colAmount) + j;
                int x = (divider * (j + 1)) - (divider / 2);
                int y = (divider * (i + 1)) - (divider / 2);
                glm::ivec2 center(x, y);
                //Fragment oluştur.
                VisionFragment temp(id, center, divider, image, &suspects, &intensityMap, &debugColors);
                fragments.push_back(temp);
            }
        }
        
        //Burada, algoritmamız için önemli olan Fragmentler'in kommşularının adres vektörlerini oluşturup,
        //atama işlemini gerçekleştiriyoruz.
        for (int i = 0; i < rowAmount; i++)
        {
            for (int j = 0; j < colAmount; j++)
            {
                std::vector<VisionFragment*> n;
                if (j != colAmount - 1)
                {
                    n.push_back(&fragments[(i * colAmount) + j + 1]);
                }
                if (j != 0)
                {
                    n.push_back(&fragments[(i * colAmount) + j - 1]);
                }
                if (i != 0)
                {
                    n.push_back(&fragments[((i - 1) * colAmount) + j]);
                }
                if (i != rowAmount - 1)
                {
                    n.push_back(&fragments[((i + 1) * colAmount) + j]);
                }
                //Çarpraz komşular
                if (i != 0 && j != 0)
                {
                    n.push_back(&fragments[((i - 1) * colAmount) + j] - 1);
                }
                if (i != 0 && j != colAmount - 1)
                {
                    n.push_back(&fragments[((i - 1) * colAmount) + j] + 1);
                }
                if (i != rowAmount - 1 && j != 0)
                {
                    n.push_back(&fragments[((i + 1) * colAmount) + j] - 1);
                }
                if (i != rowAmount - 1 && j != colAmount - 1)
                {
                    n.push_back(&fragments[((i + 1) * colAmount) + j] + 1);
                }
                //En son olarak Vision Fragment içindeki fonksiyon ile atama işlemi gerçekleştiriyoruz.
                fragments[(i * colAmount) + j].setNeigbours(n);
            }
        }

        //Fonksiyonu test etme bölgesi. Burayı ilk fırsatta böl.
        int colorCounter = 1;
        fragments[0].startObject(colorCounter);
        while (suspects.size() != 0)
        {
            colorCounter++;
            suspects[0]->startObject(colorCounter);
        }
        cout << intensityMap[0].size() << endl;
        drawBoundingBoxes(image);
        cout << "Finished" << endl;
}

    void drawBoundingBoxes(Mat image)
    {
        for (int i = 0; i < intensityMap.size(); i++)
        {
            if (intensityMap[i].at(4) > 650)
            {
                cout << "drawing" << endl;
                int minX = intensityMap[i][0] - (divider / 2);
                int maxX = intensityMap[i][1] + (divider / 2);
                int minY = intensityMap[i][2] - (divider / 2);
                int maxY = intensityMap[i][3] + (divider / 2);
                Point p1(minX, minY);
                Point p2(maxX, maxY);
                rectangle(image, p1, p2, Scalar(0, 0, 255), 4);
                imshow("process", image);
                waitKey(500);
            }
        }
    }
};
#endif
