#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>




//Chase Bowman
//cosc 3360
//10/28/2019
//Final working version of Rincon's Compression Algorithm with sockets/threads/processes
//********************************************************************************************************************************//
//Note to individual reviewing my Code:
//Appologies for the length of the code.
//To make reading easier, relevant code is from line 529 - 666
//Thanks.



struct freqAndChar
{
    int asciiValue;
    int freq;
    bool prevWritten;
};



struct charAndMsgLength
{
    int asciiValue;
    int length;
};


template<class T>
class ArrayList
{
private:
    int length = 0;
    int size = 0;
    T* list = NULL;

public:
    ArrayList();
    ArrayList(int ArraySize);
    int getLength() const;
    int getSize() const;
    ArrayList(const ArrayList& r);
    T getElementAt(int index) const;
    void addElement(T element);
    void MaxHeapPercolateUp(int nodeIndex);
    void swap(int first, int second);
    void removeElement();
    void MaxHeapPercolateDown(int nodeIndex);
    void printHeap();
    ~ArrayList();
};



template<class T>
ArrayList<T>::ArrayList() {
    size = 100;
    length = 0;
    list = new T[size];
}

template<class T>
ArrayList<T>::ArrayList(int ArraySize)
{
    if (ArraySize < 0)
        ArraySize = 100;

    size = ArraySize;
    length = 0;
    list = new T[size];
}

template<class T>
int ArrayList<T>::getSize() const
{
    return size;
}

template<class T>
int ArrayList<T>::getLength() const
{
    return length;
}

template<class T>
ArrayList<T>::ArrayList(const ArrayList& r) {
    length = r.length;
    size = r.size;
    list = new T[r.size];
    for (int i = 0; i < length; i++) {
        list[i] = r.list[i];
    }
}

template<class T>
T ArrayList<T>::getElementAt(int index) const
{
    return list[index];
}

template<class T>
void ArrayList<T>::addElement(T element) //O(nlogn)??
{
    //complete
    if (length < size)
    {
        list[length] = element; //insert at empty spot in leaf
        MaxHeapPercolateUp(length);
        ++length;
    }
    else
        std::cout << "Can't add element. Heap is full!\n";

}

template<class T>
void ArrayList<T>::MaxHeapPercolateUp(int nodeIndex)
{
    //complete
    while (nodeIndex > 0)
    {
        int parentIndex = (nodeIndex - 1) / 2;

        if (list[parentIndex].freq < list[nodeIndex].freq)
        {
            swap(parentIndex, nodeIndex);
            nodeIndex = parentIndex;
        }


        else if (list[parentIndex].freq == list[nodeIndex].freq && list[parentIndex].asciiValue > list[nodeIndex].asciiValue) //if there is a tie, the lower ascii value gets priority
        {
            swap(parentIndex, nodeIndex);
            nodeIndex = parentIndex;
        }


        else
            return;
    }
}

template <class T>
void ArrayList<T>::swap(int first, int second)
{
    T temp;
    temp = list[first];
    list[first] = list[second];
    list[second] = temp;
}

template<class T>
void ArrayList<T>::removeElement()
{
    //complete
    if (length != 0)
    {
        swap(--length, 0);
        MaxHeapPercolateDown(0);
    }

    else
        std::cout << "Heap is empty. Can't remove element!\n";

}

template<class T>
void ArrayList<T>::MaxHeapPercolateDown(int nodeIndex)
{
    //complete
    int childIndex = 2 * nodeIndex + 1;
    int value = list[nodeIndex].freq;

    //O(logn)(?)
    while (childIndex < length)
    {
        //find the max among the node and all the node's children
        int maxValue = value;
        int maxIndex = -1;

        for (int i = 0; i < 2 && i + childIndex < length; ++i) //checks left child, then right, finds max.
        {
            if (list[i + childIndex].freq > maxValue)
            {
                maxValue = list[i + childIndex].freq;
                maxIndex = i + childIndex;
            }
        }

        //If there is a tie, the lower ascii value takes precedence
        if (list[childIndex].freq == list[childIndex + 1].freq && list[childIndex].asciiValue > list[childIndex + 1].asciiValue) { maxIndex = childIndex + 1; }

        //if the root node is the largest value, do nothing
        //else, swap root with largest child value, and check all childs again
        if (maxValue == value)
            return;
        else
        {
            swap(nodeIndex, maxIndex); //NOTE: index stay the same, the values at those indexes swap.
            nodeIndex = maxIndex; //this what allows the heapify to check downwards
            childIndex = 2 * nodeIndex + 1;
        }
    }
}

template<class T>
void ArrayList<T>::printHeap()
{
    //std::cout << "Heap:\n";

    for (int i = 0; i < 256; ++i)
    {
        std::cout << " Iteration: " << i << " Symbol: " << static_cast<char>(list[i].asciiValue) << "  Frequency: " << list[i].freq << "\n";
    }

    std::cout << std::endl;
}

template<class T>
ArrayList<T>::~ArrayList() {
    delete[]list;
}

//This function get's input(due to redirection) using the .get(). It picks up EOL chars. .get returns an integer. If this int is -1,
//it matches EOF and exits the loop. The loop simply pushes the chars into a vector<char>. O(n)
std::vector<char> getUnformattedInput()
{
    std::vector<char> myChars;
    char ch;

    while ((ch = static_cast<char>(std::cin.get())) != EOF) //get() returns int. Ascii chars go from 0-255. EOF is -1. This is why ch is an int so a comparsion can be made
    {
        myChars.push_back(ch);
    }

    return myChars;
}


//This a small function to take vector<char> and puts it into a string
//O(n)
std::string convertCharToStr(std::vector<char> &myChars)
{
    std::string myStr;

    for(uint i = 0; i < myChars.size();++i)
    {
       myStr += myChars.at(i);
    }

    return myStr;
}



//This function determines the distribution of each letter in the vector<char> by use of a hashing function.
//The char and its frequency is stored in a vector<freqAndChar> where freqAndChar is a struct that contains
//three values: int representation of the char(ascii value), the frequency of that char as an integer,
//and finally a bool to indicate whether or not each value has been written to previously.
//Without the bool value, the ascii value would be written over every time in loop needlessly.
//The bool acts as a flag to minimize rewrites at the cost of initializing freqV.
//O(n) for vector initialization(?) and O(n) for finding frequency
std::vector<freqAndChar> findFrequencyOfEachLetter(const std::vector<char>& charByChar)
{
    std::vector<freqAndChar> freqV(256, { 0,0,0 }); //The size if 256 because there are 256 ascii values

    for(size_t i = 0; i < charByChar.size(); ++i)
    {
        size_t index{ static_cast<size_t>(charByChar[i])}; //Use the integer value of the ascii value to index vector.

        if(freqV[index].prevWritten == false) //If the char isn't present in vector, add it
        {
            freqV[index].asciiValue = charByChar[i];
        }

        freqV[index].freq++;

    }
    return freqV;
}


//This function creates a proper heap structure for the purpose
//of sorting the frequencies.(ascending)
//O(n) * O(nlogn) = O(n^2logn)(?)
void generateHeap(const std::vector<freqAndChar>& freqVect, ArrayList<freqAndChar>& heap)
{
    for (size_t i = 0; i < freqVect.size(); ++i) { heap.addElement(freqVect.at(i)); }
}


//In order to utilize the heap structure, the values in the heap must be removed to sort.
//O(n) * O(logn) = O(nlogn)(?)
void sortAscending(ArrayList<freqAndChar>& unsortedHeap)
{
    int lengthHeap{ unsortedHeap.getLength() };

    for (int i = 0; i < lengthHeap; ++i) { unsortedHeap.removeElement(); }
}


//Converted to vector for ease of use
//(Necessary?)O(n)
std::vector<freqAndChar> convertHeapToVector(const ArrayList<freqAndChar>& sorted)
{
    std::vector<freqAndChar> converted;

    for (int i = 0; i < sorted.getSize(); ++i) { converted.push_back(sorted.getElementAt(i)); }

    return converted;
}


//Since many char will have 0 frequencies, it is a good idea to filter those chars out
//O(n)
std::vector<freqAndChar> filterSort(ArrayList<freqAndChar>& sorted)
{
    std::vector<freqAndChar> convertedHeap{ convertHeapToVector(sorted) };
    std::vector<freqAndChar> trimmedVector;

    for (size_t i = 0; i < convertedHeap.size(); ++i)
    {
        if (convertedHeap.at(i).freq != 0) { trimmedVector.push_back(convertedHeap.at(i)); }
    }

    return trimmedVector;
}


//heap class generates max heap and therefore sorts in ascending order
//Either the heap class needs to be rewritten to generate a min heap,
//or the list can be reversed. O(n)
void reverseVector(const std::vector<freqAndChar>& ascending, std::vector<freqAndChar>& descending)
{
    size_t index{};

    for (int i = static_cast<int>(ascending.size()-1); i >= 0; --i)
    {
        descending.at(index++) = ascending.at(static_cast<size_t>(i));
    }
}


//Print the frequencies and associated chars from vector.
//O(n)
void printFrequencies(const std::vector<freqAndChar>& sorted)
{
    //std::cout << "The frequency of each symbol for the inputted data is: \n";

    for (size_t i = 0; i < sorted.size(); ++i)
    {
        if((sorted.at(i).asciiValue == 10) )//|| (sorted.at(i).asciiValue == 13)
        {
            std::cout << "<EOL> frequency = " << sorted.at(i).freq << "\n";
        }
        else
        {
            std::cout << static_cast<char>(sorted.at(i).asciiValue) << " frequency = " << sorted.at(i).freq <<"\n";
        }
    }
}


//Helper to generateDecompressionData
//Use a recursive function to determine the length of the each stripped message and stores it along with
//the ascii value into a struct in a vector.(Since the addition of multiple files containing the binary strings
//and the generation of the stripped strings, this may be able to be removed for something simpler and more readable)
//The only noteworthy thing about this recursive function is that it needs not strip the strings to generate the lengths
int calcDecomp(int dataSize, const std::vector<freqAndChar>& freqs, std::vector<charAndMsgLength>& decomp, size_t iter)
{


    if (iter != 0)
    {
        int value{ calcDecomp(dataSize, freqs, decomp, iter - 1) - freqs.at(iter - 1).freq };

        decomp.at(iter).asciiValue = freqs.at(iter).asciiValue;
        decomp.at(iter).length = value;

        return value;
    }

    else
    {
        decomp.at(0).asciiValue = freqs.at(0).asciiValue;
        decomp.at(0).length = dataSize;

        return dataSize;
    }
}


//Set up values for function that will deal will recursive calls.
//As long as the frequencies and the length of the original message are known,
//the decompression data can be generated without actually counting the stripped strings.
//The decompression data(besides the decompressed binary string) is multiple pair values: (ascii value, length of stripped message)
void generateDecompressionData(std::vector<charAndMsgLength>& decomp, const std::vector<freqAndChar>& freqs, const int dataSize)
{
    size_t iter{ freqs.size() - 1 };


    calcDecomp(dataSize, freqs, decomp, iter);

    //reverse vector to be used in decompression routine
    //Since the vector is generated by a recursive function that only works one way(I think),
    //reversing the vector is necessary. O(n/2) = O(n) (I think)
    reverse(decomp.begin(),decomp.end());
}


//Helper to filter
//Using the passed char, the string is filtered of all occurences of that char
//The used line needs review.
//Notice that the message is affected every time(it is not const) and no return value.
void removeCharsFromString(std::string& message, const char& removeChar)
{
    message.erase(std::remove(message.begin(), message.end(), removeChar), message.end());
}


//helper to stripStrs
//Strips the original message the chars stored in the frequency vector.
//Most frequent char is stripped first.
//String is stored into a vector.
void filter(std::vector<std::string>& strippedStrs, std::string& message, std::vector<freqAndChar>& freqs)
{
    strippedStrs.push_back(message); //Place original string at index 0

    for (size_t i = 0; i < freqs.size() - 1; ++i)
    {
        char charToRemove{ static_cast<char>(freqs.at(i).asciiValue) };

        removeCharsFromString(message, charToRemove);

        strippedStrs.push_back(message);
    }
}



//parent function for two other helper functions.
//sets up string for helper.
//These functions need serious review.
std::vector<std::string> stripStrs(std::vector<char> &myChars, std::vector<freqAndChar> &freqs)
{
    std::vector<std::string> strippedStrs;
    std::string message{ convertCharToStr(myChars) };

    filter(strippedStrs, message, freqs);//Strip the message of chars according to highest frequency.

    return strippedStrs;
}

//Due to the assignment requirement asking to consider EOL chars and
//Also due to the fact that printing the EOL char actually prints a newline,
//each EOL char is converted into <EOL> to keep printed output legible.
//replace() is used to replace value of EOL(10) with <EOL>
std::vector<std::string> changeEOL(std::vector<std::string> &strippedStrs)
{
    //
    std::vector<std::string> eolStrs = strippedStrs;
    std::string chg {"<EOL>"};

    for(size_t i = 0; i < eolStrs.size(); ++i)
    {
        for(size_t j = 0; j < eolStrs.at(i).size(); ++j)
        {
            if(eolStrs.at(i).at(j) == 10)
            {
                eolStrs.at(i).replace(j,1,chg);
            }
        }
    }

    return eolStrs;
}


void error(char *msg)
{
    perror(msg);
    exit(0);
}

std::vector<std::string> combineStrs(std::vector<std::string> strippedStrs, std::vector<freqAndChar> &freqs)
{
    for(int i = 0; i < strippedStrs.size(); ++i)
    {
        strippedStrs.at(i).push_back(freqs.at(i).asciiValue);
    }

    return strippedStrs;
}

struct threadArguments
{
    char binaryStr[256]; //array of binary strings
    char combinedStrs[256]; //strings to be converted
    char portNo[10];
    char hostt[10];

};

void *contactServer(void* arguments)
{
    threadArguments* argu = (threadArguments *) arguments;

    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    portno = atoi(argu->portNo);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argu->hostt);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);


    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    bzero(buffer,256);
    strcpy(buffer,argu->combinedStrs);
    n = write(sockfd,buffer,strlen(buffer)); //actually sends to server

    if (n < 0)
         error("ERROR writing to socket");
    bzero(buffer,256);

    n = read(sockfd,buffer,255);//receive data back from server

    strcpy(argu->binaryStr,buffer);
    if (n < 0)
         error("ERROR reading from socket");
}

//Print output function
void print(std::vector<freqAndChar>& freqs, threadArguments argu[], std::vector<std::string> strippedStrs)
{
    for(size_t i = 0; i < freqs.size(); ++i)
    {
        if(i == 0)
        {
            std::cout << "Original Message:       " << strippedStrs.at(i) << "\n";


            if(freqs.at(i).asciiValue == 10)
            {

                std::cout << "Symbol <EOL> code:      ";


                std::cout << argu[i].binaryStr << "\n";
            }

            else
            {
                std::cout << "Symbol " << static_cast<char>(freqs.at(i).asciiValue) << " code:          ";
                std::cout << argu[i].binaryStr << "\n";
            }
        }

        else
        {
            std::cout << "Remaining Message:      " << strippedStrs.at(i) << "\n";

            if(freqs.at(i).asciiValue == 10)
            {

                std::cout << "Symbol <EOL> code:      ";


                std::cout << argu[i].binaryStr << "\n";
            }

            else
            {
                std::cout << "Symbol " << static_cast<char>(freqs.at(i).asciiValue) << " code:          ";
                std::cout << argu[i].binaryStr << "\n";
            }

        }
    }

}

int compressData(std::vector<freqAndChar> &freqs, std::vector<std::string> strippedStrs, char port[], char host[])
{
    int numStr = strippedStrs.size();

    //array of messages with symbol added on the end. This way, I only have to send to the server once per thread
    std::vector<std::string> combinedStrs{ combineStrs(strippedStrs,freqs)};

    pthread_t tid[numStr];

    threadArguments *argStruct = new threadArguments [numStr];

    //fills structs with stripped strings and the same portNO & hostno
    for(int j = 0; j < numStr; ++j)
    {
        strcpy(argStruct[j].portNo,port);
        strcpy(argStruct[j].hostt,host);
        strcpy(argStruct[j].combinedStrs,combinedStrs.at(j).c_str());
    }


    for(int i = 0; i < numStr; ++i)
    {


        if(pthread_create(&tid[i], NULL, contactServer, &argStruct[i]))
        {
            fprintf(stderr, "Error creating thread\n");
            return 1;

        }
    }

    // Wait for the other threads to finish.
    for (int i = 0; i < numStr; i++)
        pthread_join(tid[i], NULL);



    std::vector<std::string> eolVec { changeEOL(strippedStrs) }; //takes care of <EOL> values
    print(freqs,argStruct,eolVec); //print output

    delete [] argStruct;
}







int main(int argc, char *argv[])
{
    //Get input from redirection
    std::vector<char> charByChar{ getUnformattedInput() };

    //Determine frequency of characters
    std::vector<freqAndChar> distribution{ findFrequencyOfEachLetter(charByChar) };

    //Create Heap
    ArrayList<freqAndChar> heap{ static_cast<int>(distribution.size()) };

    //Fill Heap
    generateHeap(distribution, heap);

    //Heap sort
    sortAscending(heap);

    //Chars with frequency of zero are filtered out and heap data is converted into vector
    std::vector<freqAndChar> finalList{ filterSort(heap) };

    //Prepare vector for descending values(needs to be fixed for ascending)
    std::vector<freqAndChar> descending{ finalList.size()};

    //reverse frequency array(largest to small)(5,4,3,2,1)(Uneccesary. Fix mergesort function)
    reverseVector(finalList, descending);

    //print frequencies from largest to smallest(if freqs are tied, the smallest ascii value takes precedence)
    printFrequencies(descending);

    //Delete chars from message in order to be processed by each child process
    std::vector<std::string> strippedStrs{ stripStrs(charByChar,descending) };

    //create threads and send message to server. Receive back binary code and print
    compressData(descending, strippedStrs, argv[2],argv[1]);


    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }




    return 0;
}
