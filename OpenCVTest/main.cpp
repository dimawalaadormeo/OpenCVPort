#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <fstream>
#include <sstream>


#include <stdint.h>


using namespace cv;
using namespace std;

//fucking got this from stackexchange... no time

void split(const string &s, char delim, vector<string> &elems)
{
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim))
    {
        elems.push_back(item);
    }
}


vector<string> split(const string &s, char delim)
{
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}



bool spaceCheck(const char c)
{
    return isspace(c);
}
std::string stringConvert(const char *pCh, int arraySize)
{
    std::string str;
    int start = 0;
    if(pCh[0] == '\0') start = 4;
    if (pCh[arraySize-1] == '\0') str.append(pCh);
    else for(int i=0 + start; i<arraySize; i++)
        {

            str.append(1,pCh[i]);
        }
    return str;
}

int is_big_endian(void)
{
    union
    {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

uint32_t swapByteOrder(uint32_t ui)
{
    ui = (ui >> 24) |
         ((ui<<8) & 0x00FF0000) |
         ((ui>>8) & 0x0000FF00) |
         (ui << 24);
    return ui;
}

uint32_t read_packet_length(std::istream& s)
{
    uint32_t len;
    s.read(reinterpret_cast<char*>(&len), sizeof(len));
    if (!is_big_endian())
        len = swapByteOrder(len);
    // std::cerr << 'r' << len << ' ';
    // std::cerr.flush();
    // cout << len <<endl;
    return len;
}

std::ostream&  write_packet_length(std::ostream& s, uint32_t len)
{
    //std::cerr << 'w' << len << ' ';
    // std::cerr.flush();

    if (!is_big_endian())
        len = swapByteOrder(len);
    s.write(reinterpret_cast<char*>(&len), sizeof(len));
    return s;
}

static Mat norm_0_255(InputArray _src)
{
    Mat src = _src.getMat();
    // Create and return normalized image:
    Mat dst;
    switch(src.channels())
    {
    case 1:
        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC1);
        break;
    case 3:
        cv::normalize(_src, dst, 0, 255, NORM_MINMAX, CV_8UC3);
        break;
    default:
        src.copyTo(dst);
        break;
    }
    return dst;
}

static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels,vector<string>& username, char separator = ';')
{
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file)
    {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    string acctno,uname,msisdn;

    //get first line
    getline(file,line);
    stringstream linex(line);
    getline(linex, uname,separator);
    username.push_back(uname);
    getline(linex,msisdn);
    username.push_back(msisdn);
        //get rest of lines
    while (getline(file, line))
    {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty())
        {
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}
typedef unsigned char byte;
extern "C" void read_cmd(byte *buf);
int main(int argc, const char *argv[])
{
    // Check for valid command line arguments, print usage
    // if no arguments were given.
    std::cin.exceptions ( std::istream::failbit | std::istream::badbit );
    // cout << "loading yml" << endl;
    // Ptr<FaceRecognizer> model = createEigenFaceRecognizer();
    Ptr<FaceRecognizer> model = createLBPHFaceRecognizer(1,8,8,8,123);
    string yml_file = "/home/madormeo/uhac-jpg.yml";
    // model->save("/home/madormeo/eigenfaces.yml");
    model->load(yml_file);
    ofstream myfile;
    myfile.open ("log.txt",std::ios_base::app);

    myfile << "done loading" << endl;
    myfile.close();




    while(true)
    {

        myfile.open ("log.txt",std::ios_base::app);

        myfile << "starting while" << endl;
        myfile.close();

        // read len, 4 bytes

        //  uint32_t len = read_packet_length(std::cin);
        uint32_t len = 100;
        // read data, len bytes
        char* buf = new char[len];
        std::cin.read(buf, len);
        myfile.open ("log.txt",std::ios_base::app);

        myfile << "readbuffer" << endl;
        myfile.close();

        string filePath = stringConvert(buf,len);

        vector<string> x = split(filePath,'|');
        if(x.size() == 1)
        {
            filePath.erase( remove_if(filePath.begin(),filePath.end(), spaceCheck), filePath.end());

            //  cout << filePath <<endl;

            myfile.open ("log.txt",std::ios_base::app);
            myfile << filePath<< "\n";
            myfile.close();
            Mat testFace = imread(filePath,0);

            //  Mat resizedFace ;
            //resize(testFace, resizedFace, Size(112, 92), 0, 0, INTER_CUBIC);
            int predictedLabel= -1;
            string result_message = "";

            double predicted_confidence = 0.0;
            try
            {
                model->predict(testFace,predictedLabel,predicted_confidence);
                result_message = format("%d,%d",predictedLabel,predicted_confidence);
            }
            catch (cv::Exception ex)
            {
                result_message = format("Problem with image or image does not exist in database");
            }


            char* return_message = new char[result_message.length() +1];
            strcpy(return_message, result_message.c_str());

            write_packet_length(std::cout, result_message.length() +1);

            std::cout.write(return_message, result_message.length() +1);

            std::cout.flush();

            myfile.open ("log.txt",std::ios_base::app);
            myfile << return_message<< "\n";
            myfile.close();
            delete[] buf;
            delete[] return_message;

            std::cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        //ENROLL
        else
        {
            vector<Mat> images;
            vector<int> labels;
            vector<string> username;

            try
            {


                read_csv(x.at(0), images, labels,username);

                int predictedLabel= -1;
                string result_message = "";

                double predicted_confidence = 0.0;
                try
                {
                    model->predict(images.at(0),predictedLabel,predicted_confidence);
                    //test only first image
                    string un = username.at(0);
                    string msisdn =  username.at(1);
                    result_message = format("%d,%d,%s,%s",predictedLabel,predicted_confidence,&un,&msisdn);
                    if(predicted_confidence <= -1 )
                    {

                        model->update(images,labels);
                        model->save(yml_file);

                        char* return_message = new char[result_message.length() +1];
                        strcpy(return_message, result_message.c_str());

                        write_packet_length(std::cout, result_message.length() +1);

                        std::cout.write(return_message, result_message.length() +1);

                        std::cout.flush();

                        myfile.open ("log.txt",std::ios_base::app);
                        myfile << return_message<< "\n";
                        myfile.close();
                        delete[] buf;
                        delete[] return_message;

                        std::cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                    else
                    {
                        char* return_message = "ImageExists";
                       // strcpy(return_message, result_message.c_str());

                        write_packet_length(std::cout, (unsigned)strlen(return_message) + 1);

                        std::cout.write(return_message, (unsigned)strlen(return_message) + 1);

                        std::cout.flush();

                        myfile.open ("log.txt",std::ios_base::app);
                        myfile << return_message<< "\n";
                        myfile.close();
                        delete[] buf;
                        delete[] return_message;

                        std::cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    }
                }
                catch (cv::Exception ex)
                {
                    result_message = format("Problem with image or image does not exist in database");
                }



                //cheap hack get first data



            }
            catch (cv::Exception& e)
            {
               // cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
                // nothing more we can do
                exit(1);
            }

        }
    }

    if(argc == 2)
    {
        //  waitKey(0);
    }
    return 0;
}
