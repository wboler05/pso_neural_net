#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

#define DT_UNSIGNED_TYPE 0X08
#define DT_SIGNED_BYTE 0x09
#define DT_SHORT 0X0B
#define DT_INT 0x0C
#define DT_FLOAT 0x0D
#define DT_DOUBLE 0x0E

#define CHAR_TO_INT(x) *(int *) x

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    srand(time(NULL));

    // Set the logger file
  //  Logger::setOutputFile("log/run.log");

    time_t now = time(0);
    tm *gmtm = gmtime(&now);

    std::string logFile("logs\\run_");
    logFile += stringPut(gmtm->tm_year+1900);
    logFile += stringPut(gmtm->tm_mon);
    logFile += stringPut(gmtm->tm_mday);
    logFile += "_";
    logFile += stringPut(gmtm->tm_hour);
    logFile += stringPut(gmtm->tm_min);
    logFile += stringPut(gmtm->tm_sec);
    logFile += ".log";

    cout << logFile << endl;

    // Set the logger file
    Logger::setOutputFile(logFile);

    string dateTime;
    dateTime += "\nDate / Time: ";
    dateTime += asctime(gmtm);
    dateTime += "\n";
    Logger::write(dateTime);

    std::string headerString;
    headerString += "   ****************************************\n";
    headerString += "   * PSO Neural Net (Gaussian Activation) *\n";
    headerString += "   *       by William Boler, BSCE         *\n";
    headerString += "   *     Research Assistant, IUPUI        *\n";
    headerString += "   *                                      *\n";
    headerString += "   *  Professor: Dr. Lauren Christopher   *\n";
    headerString += "   *     Created: March 13, 2017          *\n";
    headerString += "   ****************************************\n";
    Logger::write(headerString);


    cl::Context _context;

    initializeCL(_cpuDevices, _gpuDevices, _allDevices);

    //! TODO Need to change this functionality
    //boost::thread thread1(&this->runNeuralPso);
    //boost::thread thread2(&this->onKeyInput);
    runNeuralPso();
    onKeyInput();

    //thread1.join();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onKeyInput() {
  try {
    while(true) {
      int in = cin.get();//getchar();

      if ((char)in == 'c') {
        cout << "Ending process.  Please wait. " << endl;
        NeuralPso::interruptProcess();
        return;
      } else
      if ((char) in == 'p') {
        NeuralPso::setToPrint();
      }
      if ((char) in == 'g') {
        NeuralPso::setToPrintGBNet();
      }

    }
  } catch (boost::thread_interrupted &) {
    cout << "Project complete!" << endl;
    return;
  }

}

void MainWindow::runNeuralPso() {
  /// image.rows.cols
  //vector<vector<vector<uint8_t> > > trainingImages;
  //vector<uint8_t> trainingLabels;

  vector<double> labels;
  vector<vector<double>> input;

  if (!readPEFile(labels, input)) {
    return;
  }

  PsoParams pParams;
  pParams.particles = 50; // 50
  pParams.neighbors = 10; // 10
  pParams.iterations = 1000;
  pParams.delta = 5E-6;
  pParams.vDelta = 5E-200;
  pParams.termIterationFlag = true;
  pParams.termDeltaFlag = false;

  /*
  NeuralNetParameters nParams;
  nParams.inputs = trainingImages[0].size() * trainingImages[0][0].size();
  nParams.innerNets = 1;
  nParams.innerNetNodes.push_back(100);
  //nParams.innerNetNodes.push_back(50);
  nParams.outputs = 10;
  */
  NeuralNetParameters nParams;
  nParams.inputs = input[0].size();
  nParams.innerNetNodes.push_back(3); // 8
  //nParams.innerNetNodes.push_back(10);
  //nParams.innerNetNodes.push_back(4);
  nParams.innerNets = nParams.innerNetNodes.size();
  nParams.outputs = 2;
  nParams.testIterations = 500;

  std::string outputString;

  outputString += "\n\nInputs: ";
  outputString += stringPut(nParams.inputs);
  outputString += "\nInner Nets: ";
  outputString += stringPut(nParams.innerNets);
  outputString += "\n";
  for (uint i = 0; i < nParams.innerNetNodes.size(); i++) {
    outputString += " - ";
    outputString += stringPut(nParams.innerNetNodes[i]);
    outputString += "\n";
    //cout << " - " << nParams.innerNetNodes[i] << endl;
  }
  outputString += "Tests per train(min): ";
    outputString += stringPut(nParams.testIterations);
    outputString += "\n";
  outputString += "Particles: ";
    outputString += stringPut(pParams.particles);
    outputString += "\nNeighbors: ";
    outputString += stringPut(pParams.neighbors);
    outputString += "\n";
  outputString += "Minimum Particle Iterations: ";
    outputString += stringPut(pParams.iterations);
    outputString += "\n";

  //cout << "Tests per train(min): " << nParams.testIterations << endl;
  //cout << "Particles: " << pParams.particles << "\nNeighbors: " << pParams.neighbors << endl;
  //cout << "Minimum Particle Iterations: " << pParams.iterations << endl;
  Logger::write(outputString);

  for (uint i = 0; i < labels.size(); i++) {
    if (labels[i] != 1) {
      labels[i] = 0;
    }
  }

  NeuralPso *np = new NeuralPso(pParams, nParams);
  //np->build(trainingImages, trainingLabels);
  np->build(input, labels);
  np->setFunctionMsg("PE");

  NeuralNet *net = np->neuralNet();

  // Train this shit
  np->runTrainer();

  /*
  for (int j = 0; j < 15; j++) { // Test point
    np->testGB();
  }
  */

  np->classError();

  return;

}



void MainWindow::loadTrainingData(string imageFile, string labelFile, vector<vector<vector<uint8_t> > > &trainingImages, vector<uint8_t> &trainingLabels) {
  ifstream trainLabelFile(labelFile, ios::binary);

  uint8_t temp;
  LabelInfo trainingLabelInfo;

  if (readLabelHeading(trainLabelFile, trainingLabelInfo)) {
    cout << "Data type: " << trainingLabelInfo.dataType
      << "\tDimensions: " << trainingLabelInfo.dataDimensions << "\tNum items: " << trainingLabelInfo.numItems << endl;
  }

  trainingLabels.resize(trainingLabelInfo.numItems);

  for (uint32_t i = 0; i < trainingLabelInfo.numItems; i++) {
    if (trainLabelFile.eof()) {
      cout << "You can't count, son! " << trainingLabelInfo.numItems << " != " << i << endl;
      break;
    } else {
      temp = trainLabelFile.get();
      trainingLabels[i] = temp;
      //cout << "(" << i << "): " << (uint32_t) trainingLabels.at(i) << endl;
    }
  }
  trainLabelFile.close();


  ifstream trainImageFile(imageFile, ios::binary);
  // Load training images
  ImageInfo trainingImageInfo;
  if (readImageHeading(trainImageFile, trainingImageInfo)) {
    cout << "Image file read. " << endl;
  } else {
    cout << "Image file broke, yo." << endl;
  }

  trainingImages.resize(trainingImageInfo.numItems);
  for (uint i = 0; i < trainingImageInfo.numItems; i++) {
    trainingImages[i].resize(trainingImageInfo.rows);
    for (uint j = 0; j < trainingImageInfo.rows; j++) {
      trainingImages[i][j].resize(trainingImageInfo.cols);
    }
  }

  cout << "Allocated image space." << endl;

  for (uint i = 0; i < trainingImageInfo.numItems; i++) {
    for (uint j = 0; j < trainingImageInfo.rows; j++) {
      for (uint k = 0; k < trainingImageInfo.cols; k++) {
        temp = trainImageFile.get();
        trainingImages[i][j][k] = temp;
      }
    }
  }
  trainImageFile.close();

  cout << "Loaded training images." << endl;

}

bool MainWindow::readPEFile(vector<double> &labels, vector<vector<double>> &data) {

  ifstream inputFile("FNNPSOGSAclot/clean.csv");

  // Select data columns to skip (never select '0'th column!)
  vector<int> skips = {3, 5, 6, 8};

  if (!inputFile.is_open()) {
    cout << "File could not be read." << endl;
    return false;
  }

  labels.clear();
  data.clear();

  string temp;
  getline(inputFile, temp, '\n');

  while (getline(inputFile, temp)) {

    vector<double> inData;
    string::size_type prevP = 0, pos = 0;
    int index = 0;
    while ((pos = temp.find(',', pos)) != std::string::npos) {
      bool skipLine = false;
      for (uint i = 0; i < skips.size(); i++) {
        if (skips[i] == 0) continue;
        if (index == skips[i]) {
          skipLine = true;
          break;
        }
      }
      index++;
      if (!skipLine) {
        std::string substring(temp.substr(prevP, pos-prevP));

        double dig;
        {
          stringstream ss;
          ss << substring;
          ss >> dig;
        }
        inData.push_back(dig);
      }
      prevP = ++pos;
    }
    labels.push_back(inData[0]);
    vector<double> data_;
    data_.resize(inData.size()-1);
    for(size_t i = 1; i < inData.size(); i++) {
      data_[i-1] = inData[i];
    }
    data.push_back(data_);
  }
  inputFile.close();
  return true;
}








//bool MainWindow::readMagicNumber(ifstream &in, uint32_t &dataType, uint32_t &dimensions, uint32_t &numItems) {}
bool MainWindow::readLabelHeading(ifstream &in, LabelInfo &lb) {

  in.seekg(0, in.beg);

  unsigned char temp;

  for (int i = 0; i < 2; i++) {
      temp = in.get();
      if (temp != 0)
        return false;
  }

  lb.dataType = (uint32_t) in.get();
  lb.dataDimensions = (uint32_t) in.get();

  lb.numItems = readUnsignedInt(in);

  return true;
}

bool MainWindow::readImageHeading(ifstream &in, ImageInfo &im) {
  in.seekg(0, in.beg);

  unsigned char temp;

  for (int i = 0; i < 2; i++) {
    temp = in.get();
    if (temp != 0)
      return false;
  }

  im.dataType = (uint32_t) in.get();
  im.dataDimensions = (uint32_t) in.get();

  im.numItems = readUnsignedInt(in);
  im.rows = readUnsignedInt(in);
  im.cols = readUnsignedInt(in);

  return true;
}

uint32_t MainWindow::readUnsignedInt(ifstream &input) {
  uint8_t temp[4];
  for (int i = 3; i >= 0; i--) {
    temp[i] = input.get();
  }
  return char2uint(temp);
}

uint32_t MainWindow::char2uint(uint8_t *input) {
  uint32_t num = 0x00000000;
  num +=  ((uint32_t) input[0])         & 0x000000FF;
  num +=  (((uint32_t) input[1]) << 8)  & 0x0000FF00;
  num +=  (((uint32_t) input[2]) << 16) & 0x00FF0000;
  num +=  (((uint32_t) input[3]) << 24) & 0xFF000000;
  return num;
}

void MainWindow::initializeCL(std::vector<cl::Device> &cpuDevices,
                  std::vector<cl::Device> &gpuDevices,
                  std::vector<cl::Device> &allDevices)
{

    std::vector<cl::Platform> platforms;

    cl::Platform::get(&platforms);

    for (uint i = 0; i < platforms.size(); i++) {
        std::vector<cl::Device> devices;
        platforms[i].getDevices(CL_DEVICE_TYPE_CPU, &devices);
        for (uint j = 0; j < devices.size(); j++) {
            cpuDevices.push_back(devices.at(j));
            allDevices.push_back(devices.at(j));
        }
    }

    for (uint i = 0; i < platforms.size(); i++) {
        std::vector<cl::Device> devices;
        platforms[i].getDevices(CL_DEVICE_TYPE_GPU, &devices);
        for (uint j = 0; j < devices.size(); j++) {
            gpuDevices.push_back(devices.at(j));
            allDevices.push_back(devices.at(j));
        }
    }

    cout << "C++ OpenCL.  Devices found: " << allDevices.size() << endl;
    cout << " - Total CPU Devices: " << cpuDevices.size() << endl;
    for (uint i = 0; i < cpuDevices.size(); i++) {
        cl::Device &dev = cpuDevices[i];
        std::string deviceName;
        std::string deviceVendor;
        std::string deviceVersion;
        dev.getInfo(CL_DEVICE_NAME, &deviceName);
        dev.getInfo(CL_DEVICE_VENDOR, &deviceVendor);
        dev.getInfo(CL_DEVICE_VERSION, &deviceVersion);
        cout << " -- " << deviceName.c_str() << "\t " << deviceVendor.c_str() << "\t" << deviceVersion.c_str() << endl;
    }
    cout << " - Total GPU Devices: " << gpuDevices.size() << endl;
    for (uint i = 0; i < gpuDevices.size(); i++) {
        cl::Device &dev = gpuDevices[i];
        std::string deviceName;
        std::string deviceVendor;
        std::string deviceVersion;
        dev.getInfo(CL_DEVICE_NAME, &deviceName);
        dev.getInfo(CL_DEVICE_VENDOR, &deviceVendor);
        dev.getInfo(CL_DEVICE_VERSION, &deviceVersion);
        cout << " -- " << deviceName.c_str() << "\t " << deviceVendor.c_str() << "\t" << deviceVersion.c_str() << endl;
    }
}


/*

FILE FORMATS FOR THE MNIST DATABASE

The data is stored in a very simple file format designed for storing vectors and multidimensional matrices. General info on this format is given at the end of this page, but you don't need to read that to use the data files.
All the integers in the files are stored in the MSB first (high endian) format used by most non-Intel processors. Users of Intel processors and other low-endian machines must flip the bytes of the header.

There are 4 files:

train-images-idx3-ubyte: training set images
train-labels-idx1-ubyte: training set labels
t10k-images-idx3-ubyte:  test set images
t10k-labels-idx1-ubyte:  test set labels

The training set contains 60000 examples, and the test set 10000 examples.

The first 5000 examples of the test set are taken from the original NIST training set. The last 5000 are taken from the original NIST test set. The first 5000 are cleaner and easier than the last 5000.

TRAINING SET LABEL FILE (train-labels-idx1-ubyte):

[offset] [type]          [value]          [description]
0000     32 bit integer  0x00000801(2049) magic number (MSB first)
0004     32 bit integer  60000            number of items
0008     unsigned byte   ??               label
0009     unsigned byte   ??               label
........
xxxx     unsigned byte   ??               label
The labels values are 0 to 9.

TRAINING SET IMAGE FILE (train-images-idx3-ubyte):

[offset] [type]          [value]          [description]
0000     32 bit integer  0x00000803(2051) magic number
0004     32 bit integer  60000            number of images
0008     32 bit integer  28               number of rows
0012     32 bit integer  28               number of columns
0016     unsigned byte   ??               pixel
0017     unsigned byte   ??               pixel
........
xxxx     unsigned byte   ??               pixel
Pixels are organized row-wise. Pixel values are 0 to 255. 0 means background (white), 255 means foreground (black).

TEST SET LABEL FILE (t10k-labels-idx1-ubyte):

[offset] [type]          [value]          [description]
0000     32 bit integer  0x00000801(2049) magic number (MSB first)
0004     32 bit integer  10000            number of items
0008     unsigned byte   ??               label
0009     unsigned byte   ??               label
........
xxxx     unsigned byte   ??               label
The labels values are 0 to 9.

TEST SET IMAGE FILE (t10k-images-idx3-ubyte):

[offset] [type]          [value]          [description]
0000     32 bit integer  0x00000803(2051) magic number
0004     32 bit integer  10000            number of images
0008     32 bit integer  28               number of rows
0012     32 bit integer  28               number of columns
0016     unsigned byte   ??               pixel
0017     unsigned byte   ??               pixel
........
xxxx     unsigned byte   ??               pixel
Pixels are organized row-wise. Pixel values are 0 to 255. 0 means background (white), 255 means foreground (black).






the IDX file format is a simple format for vectors and multidimensional matrices of various numerical types.
The basic format is

magic number
size in dimension 0
size in dimension 1
size in dimension 2
.....
size in dimension N
data

The magic number is an integer (MSB first). The first 2 bytes are always 0.

The third byte codes the type of the data:
0x08: unsigned byte
0x09: signed byte
0x0B: short (2 bytes)
0x0C: int (4 bytes)
0x0D: float (4 bytes)
0x0E: double (8 bytes)

The 4-th byte codes the number of dimensions of the vector/matrix: 1 for vectors, 2 for matrices....

The sizes in each dimension are 4-byte integers (MSB first, high endian, like in most non-Intel processors).

The data is stored like in a C array, i.e. the index in the last dimension changes the fastest.

*/

