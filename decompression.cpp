#include <iostream>
#include <pthread.h>
#include <vector>

static pthread_mutex_t sem;
static pthread_mutex_t sem2;
static pthread_cond_t myturn = PTHREAD_COND_INITIALIZER;

//This program takes decompresses data produced for Rincon's published compression procedure
//Assuming the input file is of a certain format, this program can decompress the message using multiple threads.
//However, it should be noted that even though multiple threads are being used, this does not improve the performance
//of the algorithm. Since the decompressed string is built little by little and the next thread requires
// the previous thread to finished, making this application multi-threaded is not in any way a form of optimization.
//It simply serves as a platform to use to threads. Semaphores are used to oversee
//process synchronization.

struct input
{
    std::vector<std::string> compressedBinStr;
    std::vector<std::string> decompressedChars;
};

struct sharedData
{
    std::string binaryStr;
    std::string compressedChar;

    std::string decompressedData;

    int numOfUnqCh;
    int maxNum;
};

void parseData(std::string compressedStr,input* inputStruct)
{

    size_t found = compressedStr.find("  ");
    if(found!= std::string::npos)
    {
        inputStruct->decompressedChars.push_back(" ");
        compressedStr.erase(0,2);
        inputStruct->compressedBinStr.push_back(compressedStr);

        return;
    }

    found = compressedStr.find("<EOL>");
    if(found!= std::string::npos)
    {
        inputStruct->decompressedChars.push_back("\n");
        compressedStr.erase(0,6);
        inputStruct->compressedBinStr.push_back(compressedStr);

        return;
    }

    else
    {
        std::string ch { compressedStr.at(0)};
        inputStruct->decompressedChars.push_back(ch);
        //std::cout << "Inside parse. Size is: " << inputStruct->decompressedChars.size() << "\n";
        compressedStr.erase(0,2);
        inputStruct->compressedBinStr.push_back(compressedStr);

        return;
    }
}

void getUnformattedInput(input* inputStruct)
{
    std::string line;

    while(getline(std::cin,line))
    {
        //std::cout << "GOT LINE: " << line << "\n";
        parseData(line,inputStruct);
    }
}

void* decompress(void *void_ptr)
{
    //Set local ptr.
    sharedData *dataPtr = static_cast<sharedData*>(void_ptr);

    std::string localBin { dataPtr->binaryStr};
    std::string localCh { dataPtr->compressedChar};
    int turn { dataPtr->numOfUnqCh};

    pthread_mutex_unlock(&sem);

    pthread_mutex_lock(&sem2);

    while(turn!= dataPtr->maxNum)
    {
        pthread_cond_wait(&myturn, &sem2);
    }


    size_t EOL = localCh.find(static_cast<char>(10));

    if(EOL != std::string::npos)
        std::cout << "<EOL> Binary code = " << localBin << "\n";
    else
        std::cout << localCh << " Binary code = " << localBin << "\n";

    for(size_t i = 0; i < localBin.size(); ++i)
    {
        if(localBin.at(i) == '1')
        {
            dataPtr->decompressedData.insert(i,localCh);//compressionData.decompressChar
        }
    }

    --dataPtr->maxNum;

    //wake up sleeping threads
    pthread_cond_broadcast(&myturn);
    pthread_mutex_unlock(&sem2);
}


int main()
{
    input myInput;
    sharedData data;

    getUnformattedInput(&myInput);

    //The number of threads is equal to the number of lines stored in vector
    int numberOfThreads{static_cast<int>(myInput.decompressedChars.size())};

    //Array of threads
    pthread_t tid[numberOfThreads];

    //Sets mutex to 1
    pthread_mutex_init(&sem, NULL);
    pthread_mutex_init(&sem2, NULL);


    //Create threads and pass in struct
    for(int i=0;i<numberOfThreads;i++)
    {
        pthread_mutex_lock(&sem);

        data.binaryStr = myInput.compressedBinStr.at(i);
        data.compressedChar = myInput.decompressedChars.at(i);
        data.numOfUnqCh = i;
        data.maxNum = numberOfThreads - 1;

        if(pthread_create(&tid[i], NULL, decompress,(void *)&data))
        {
            std::cerr << "Error creating thread\n";
            return 1;
        }
    }

    //Wait for threads to finish.
    for (int i = 0; i < numberOfThreads; i++)
            pthread_join(tid[i], NULL);


    std::cout <<"Decompressed file contents:\n" << data.decompressedData << "\n";

    return 0;
}

