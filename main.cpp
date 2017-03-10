#include <iostream>
#include <fstream>
#include <cinttypes>
#include <vector>
using namespace std;

#include "NeuralNet/NeuralNet.h"

#define DT_UNSIGNED_TYPE 0X08
#define DT_SIGNED_BYTE 0x09
#define DT_SHORT 0X0B
#define DT_INT 0x0C
#define DT_FLOAT 0x0D
#define DT_DOUBLE 0x0E

#define CHAR_TO_INT(x) *(int *) x

struct LabelInfo {
  uint32_t dataType;
  uint32_t dataDimensions;
  uint32_t numItems;
};

struct ImageInfo {
  uint32_t dataType;
  uint32_t dataDimensions;
  uint32_t numItems;
  uint32_t rows;
  uint32_t cols;
};

void loadTrainingData(string imageFile, string labelFile, vector<vector<vector<uint8_t> > > &trainingImages, vector<uint8_t> &trainingLabels);
bool readMagicNumber(ifstream &in, uint32_t &dataType, uint32_t &dimensions, uint32_t &numItems);
bool readLabelHeading(ifstream &in, LabelInfo &lb);
bool readImageHeading(ifstream &in, ImageInfo &im);
uint32_t char2uint(uint8_t *input);
uint32_t readUnsignedInt(ifstream &input);

int main() {
  srand(time(NULL));


  /// image.rows.cols
  vector<vector<vector<uint8_t> > > trainingImages;
  vector<uint8_t> trainingLabels;


  string trainImageInputFile("train-images.idx3-ubyte");
  string trainLabelInputFile("train-labels.idx1-ubyte");
  string testImageInputFile("t10k-images.idx3-ubyte");
  string testLabelInputFile("t10k-labels.idx1-ubyte");

  loadTrainingData(trainImageInputFile,
                        trainLabelInputFile,
                        trainingImages,
                        trainingLabels);

  if (trainingImages.size() == 0) {
    cout << "You messed up. Images are empty." << endl;
    return 0;
  }

  NeuralNet *net = new NeuralNet();
  net->setTotalInputs(trainingImages[0].size() * trainingImages[0][0].size());
  net->setTotalInnerNets(2);
  net->setInnerNetNodes(100, 0);
  net->setInnerNetNodes(50, 1);
  net->setTotalOutputs(10);
  net->trainData(trainingImages, trainingLabels);
  net->buildNets();

  //net->resetInputs();

  for (uint i = 0; i < trainingImages[0].size(); i++) {
    int N = trainingImages[0].size();
    for (uint j = 0; j < trainingImages[0][0].size(); j++) {
      byte socrates = trainingImages[0][i][j];
      uint plato = N*i + j;
      net->loadInput(((double) socrates) / 255.0, plato);
    }
  }
  vector<double> result = net->process();

  cout << "Label: " << (uint32_t) trainingLabels[0] << endl;
  cout << "Net output: " << endl;

  for (uint i = 0; i < result.size(); i++) {
    cout << " (" << i << ") - " << result.at(0) << endl;
  }
  cout << endl;

  return 0;

}



void loadTrainingData(string imageFile, string labelFile, vector<vector<vector<uint8_t> > > &trainingImages, vector<uint8_t> &trainingLabels) {
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








//bool readMagicNumber(ifstream &in, uint32_t &dataType, uint32_t &dimensions, uint32_t &numItems) {}
bool readLabelHeading(ifstream &in, LabelInfo &lb) {

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

bool readImageHeading(ifstream &in, ImageInfo &im) {
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

uint32_t readUnsignedInt(ifstream &input) {
  uint8_t temp[4];
  for (int i = 3; i >= 0; i--) {
    temp[i] = input.get();
  }
  return char2uint(temp);
}

uint32_t char2uint(uint8_t *input) {
  uint32_t num = 0x00000000;
  num +=  ((uint32_t) input[0])         & 0x000000FF;
  num +=  (((uint32_t) input[1]) << 8)  & 0x0000FF00;
  num +=  (((uint32_t) input[2]) << 16) & 0x00FF0000;
  num +=  (((uint32_t) input[3]) << 24) & 0xFF000000;
  return num;
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
