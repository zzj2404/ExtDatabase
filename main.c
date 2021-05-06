#include <stdlib.h>
#include <stdio.h>
#include "extmem.h"
#define BLK_FULL 0

int maxUsedAddr = 48;

//Q1
void linearselect(Buffer *buf);

//Q2
void mergeR(Buffer *buf);
void mergeS(Buffer *buf);
void mergestage1(Buffer *buf,int startAddr, int endAddr);
void mergestage2(Buffer *buf,int startAddr, int endAddr);
void mergeBlksInBuffer(unsigned char *setblk[],int size,unsigned char *outputblk,Buffer *buf);

//Q3
void index(Buffer *buf,int startAddr);
void selectInIndex(int startAddr,int X,int Y,Buffer *buf);

//Q4
void innerjoin(Buffer *buf);

//Q5
void and(Buffer *buf);

//output
int writeoutput(int X,int Y,unsigned char *blk);
void writeNxtAddr(unsigned char *blk,int nextAddr);
void outputEnd(unsigned char *blk,int current,Buffer *buf);

//Buffer utils
void testBuffer(unsigned char *blk[], int size);
void sortblk(unsigned char *blk, Buffer *buf);
int getX(unsigned char *blk,int index);
int getY(unsigned char *blk,int index);
void setX(unsigned char *blk,int i,int X);
void setY(unsigned char *blk,int i,int Y);
void num2str(char str[5],int num);

//print block
void testContent(int addr,Buffer *buf);

int main(int argc, char **argv) {

    Buffer buf; /* A buffer */
    int i = 0;
    int preIO=0;

    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }


//    linearselect(&buf);
//    printf("IO time:%d\n",buf.numIO-preIO);
//
//    testContent(49,&buf);
//    testContent(50,&buf);


    mergeR(&buf);
    mergeS(&buf);
    index(&buf,65);//R index地址145-147
    for (int i=145;i<=147;i++){
        testContent(i,&buf);
    }
    index(&buf,113);//S index地址148-152
    for (int i=148;i<=152;i++){
        testContent(i,&buf);
    }

    preIO = buf.numIO;
    selectInIndex(148,50,0,&buf);//结果在153-154
    printf("IO time:%d\n",buf.numIO-preIO);
    for (int i = 153;i<=154;i++){
        testContent(i,&buf);
    }

    //可能前面内存没操作好，不想debug了
    freeBuffer(&buf);
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    //

    innerjoin(&buf);
    for (int i = 155;i<=250;i++){
        testContent(i,&buf);
    }

    freeBuffer(&buf);
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    and(&buf);
    for (int i = 251;i<=296;i++){
        testContent(i,&buf);
    }

    printf("max addr:%d\n",maxUsedAddr);
    return 0;
}

void linearselect(Buffer *buf){
    unsigned char *outputblk= getNewBlockInBuffer(buf);
    int current;
    for (int i =17;i<=48;i++){
        unsigned char *blk;
        blk = readBlockFromDisk(i, buf);
        for (int j=0;j<7;j++){
            int X = getX(blk,j);
            int Y = getY(blk,j);
            if (X==50){
                //printf("S.C=%d,S.D=%d\n",X,Y);
                current = writeoutput(X,Y,outputblk);
                if (current == BLK_FULL){
                    writeBlockToDisk(outputblk,maxUsedAddr,buf);
                    outputblk = getNewBlockInBuffer(buf);
                }
            }
        }
        freeBlockInBuffer(blk,buf);
    }
    outputEnd(outputblk,current,buf);
    freeBlockInBuffer(outputblk,buf);

//    testContent(49,buf);
//    testContent(50,buf);
}
void mergeR(Buffer *buf){
    mergestage1(buf, 1 , 16);
    for (int i = 49;i<=64;i++){//49-64一阶段结果
        testContent(i,buf);
    }
    mergestage2(buf,49, 64);
    for (int i = 65;i<=80;i++){//65-80二阶段结果
        testContent(i,buf);
    }
}
void mergeS(Buffer *buf){
    mergestage1(buf, 17 , 48);
    for (int i = 81;i<=112;i++){//81-112一阶段结果
        testContent(i,buf);
    }
    mergestage2(buf,81, 112);
    for (int i = 113;i<=144;i++){//113-144二阶段结果
        testContent(i,buf);
    }
}

void writeNxtAddr(unsigned char *blk,int nextAddr){
    char strNxtAddr[5];
    num2str(strNxtAddr,nextAddr);
    for (int k=0;k<4;k++){
        *(blk + 7*8 + k)=strNxtAddr[k];
    }
}
int writeoutput(int X,int Y,unsigned char *blk){
    static int p = 0;
    char strX[5];
    char strY[5];
    num2str(strX,X);
    num2str(strY,Y);
    for (int k=0;k<4;k++){
        *(blk + p*8 + k)=strX[k];
        *(blk + p*8 + 4 + k)=strY[k];
    }
    //printf("X=%d,Y=%d\n",X,Y);
    p++;
    if (p==7){
        p=0;
        maxUsedAddr++;
        writeNxtAddr(blk,maxUsedAddr + 1);
    }
    return p;
}
void outputEnd(unsigned char *blk,int current,Buffer *buf){
    if (current != BLK_FULL){
        while (writeoutput(0,0,blk)!=BLK_FULL);
    }
    else{
        freeBlockInBuffer(blk,buf);
        blk = readBlockFromDisk(maxUsedAddr, buf);
    }
    writeNxtAddr(blk,0);
    writeBlockToDisk(blk,maxUsedAddr,buf);
}

void num2str(char str[5],int num){
    int p = 0;
    int ten = 1000;
    int m=0,n=num,flag=1;
    for (int k=0;k<4;k++){
        m = n/ten;
        n = n-m*ten;
        if ((m >0 && flag)||!flag){
            str[p++] = (char)('0'+m);
            flag=0;
        }
        ten/=10;
    }
    while (p<5){
        str[p]='\0';
        p++;
    }
}
int getX(unsigned char *blk,int index){
    char str[5];
    for (int k = 0; k < 4; k++)
    {
        str[k] = *(blk + index*8 + k);
    }
    //printf("X:%s\n",str);
    return atoi(str);
}
int getY(unsigned char *blk,int index){
    char str[5];
    for (int k = 0; k < 4; k++)
    {
        str[k] = *(blk + index*8 + 4 + k);
    }
    //printf("X:%s\n",str);
    return atoi(str);
}
void setX(unsigned char *blk,int i,int X){
    char strX[5];
    num2str(strX,X);
    for (int k=0;k<4;k++){
        *(blk + i*8 + k)=strX[k];
    }
}
void setY(unsigned char *blk,int i,int Y){
    char strY[5];
    num2str(strY,Y);
    for (int k=0;k<4;k++){
        *(blk + i*8 + 4 + k)=strY[k];
    }
}
int getNxtAddr(unsigned char *blk){
    char str[5];
    for (int k = 0; k < 4; k++)
    {
        str[k] = *(blk + 7*8 + k);
    }
    return atoi(str);
}
void testContent(int addr,Buffer *buf){
    unsigned char *blk = readBlockFromDisk(addr, buf);
    printf("----block%d-----\n",addr);
    for (int i =0;i<7;i++){
        printf("X:%d,Y:%d\n", getX(blk,i), getY(blk,i));
    }
    printf("next addr:%d\n",atoi(blk + 7*8));
    printf("----------------\n");
    freeBlockInBuffer(blk,buf);
}
void testBuffer(unsigned char *blk[], int size) {
    for (int i=0;i<size;i++){
        printf("----buffer%d-----\n",i);
        for (int j=0;j<7;j++){
            printf("X:%d,Y:%d\n", getX(blk[i],j),getY(blk[i],j));
        }
        printf("----------------\n");
    }
}
void mergeBlksInBuffer(unsigned char *setblk[],int size,unsigned char *outputblk,Buffer *buf){
    int p[size];
    for (int i=0;i<size;i++){
        p[i] = 0;
    }

    int current;
    int endcount = 0;
    while (endcount != size){//归并
        //testBuffer(setblk, 3);
        int min = 9999;
        int minb = 0;
        for (int k=0;k<size;k++){//从集合的每个块中选出最小的X
            if (p[k]<7){
                int toSort = getX(setblk[k],p[k]);
                if (toSort<min){
                    min = toSort;
                    minb = k;
                }
            }
        }
        //printf("in blk %d,min=%d\n",minb,min);
        current = writeoutput(
                getX(setblk[minb],p[minb]),
                getY(setblk[minb],p[minb]),
                outputblk);
        if (current == BLK_FULL){
            writeBlockToDisk(outputblk,maxUsedAddr,buf);
            outputblk = getNewBlockInBuffer(buf);
        }

        p[minb]++;
        if (p[minb]==7){
            endcount++;
        }
    }
    outputEnd(outputblk,current,buf);
}
void mergestage1(Buffer *buf,int startAddr, int endAddr){
    int setSize = (endAddr - startAddr)/6 + 1;

    unsigned char *setblk[setSize];
    unsigned char *outputblk = getNewBlockInBuffer(buf);

    int i = startAddr;

    while (i+setSize-1<=endAddr){
        for (int j=i;j<i+setSize;j++){//对集合中的各个块排序
            setblk[j-i]= readBlockFromDisk(j,buf);
            sortblk(setblk[j-i],buf);
        }
        mergeBlksInBuffer(setblk,setSize,outputblk,buf);

        freeBlockInBuffer(outputblk, buf);
        for (int k=0;k<setSize;k++){
            freeBlockInBuffer(setblk[k],buf);
        }
        outputblk = getNewBlockInBuffer(buf);

        i+=setSize;
    }

    for (int j=i;j<=endAddr;j++){//对集合中的各个块排序
        setblk[j-i]= readBlockFromDisk(j,buf);
        sortblk(setblk[j-i],buf);
    }
    mergeBlksInBuffer(setblk,endAddr - i + 1,outputblk,buf);

    for (int k=0;k<endAddr-i+1;i++){
        freeBlockInBuffer(setblk[k],buf);
    }
    freeBlockInBuffer(outputblk,buf);
}
void mergestage2(Buffer *buf,int startAddr, int endAddr){
    int setSize = (endAddr - startAddr)/6 + 1;

    int setp[6];//记录每个集合当前使用块的地址
    int blkp[6];//记录内存中每块到第几个元组
    unsigned char *setblk[6];
    unsigned char *outputblk = getNewBlockInBuffer(buf);
    for (int i=0;i<6;i++){//首先将每个集合的第一块装入内存
        setp[i]=startAddr + i*setSize;
        setblk[i] = readBlockFromDisk(setp[i],buf);
        blkp[i]=0;
    }

    int current;
    int endcount = 0;
    while (endcount != 6) {//归并
        int min = 9999;
        int minb = 0;
        for (int k = 0; k < 6; k++) {//从集合的每个块中选出最小的X
            if (blkp[k] < 7) {
                int toSort = getX(setblk[k], blkp[k]);
                if (toSort < min) {
                    min = toSort;
                    minb = k;
                }
            }
        }
        current = writeoutput(
                getX(setblk[minb], blkp[minb]),
                getY(setblk[minb], blkp[minb]),
                outputblk);
        if (current == BLK_FULL) {
            writeBlockToDisk(outputblk, maxUsedAddr, buf);
            outputblk = getNewBlockInBuffer(buf);
        }

        blkp[minb]++;
        if (setp[minb] != 0) {
            if (blkp[minb] == 7) {//一个块用完重新装入
                setp[minb] = getNxtAddr(setblk[minb]);
                freeBlockInBuffer(setblk[minb], buf);
                //如果free某一块后不重新申请输出块，内存就会错乱，搞不懂都
                freeBlockInBuffer(outputblk,buf);
                outputblk = getNewBlockInBuffer(buf);
                //
                printf("next %d\n",setp[minb]);
                if (setp[minb] != 0) {//如果没用完才装入下一块
                    setblk[minb] = readBlockFromDisk(setp[minb], buf);
                    blkp[minb] = 0;
                }
                else{//如果用完，Blkp不会重置，在归并时对应集合就会被跳过
                    endcount++;
                }
            }
        }
    }
    outputEnd(outputblk,current,buf);
}
void index(Buffer *buf,int startAddr){
    unsigned char *outputblk = getNewBlockInBuffer(buf);
    unsigned char *blk;
    int current;
    while(startAddr != 0){
        blk = readBlockFromDisk(startAddr,buf);
        current = writeoutput(getX(blk,0),startAddr,outputblk);
        if (current == BLK_FULL) {
            writeBlockToDisk(outputblk, maxUsedAddr, buf);
            outputblk = getNewBlockInBuffer(buf);
        }
        startAddr = getNxtAddr(blk);
        printf("next addr:%d\n",startAddr);
        freeBlockInBuffer(blk,buf);
    }
    outputEnd(outputblk,current,buf);
    freeBlockInBuffer(outputblk,buf);
}
void selectInIndex(int startAddr,int X,int Y,Buffer *buf){
    unsigned char *outputblk = getNewBlockInBuffer(buf);
    unsigned char *indexblk[5];
    int indexsize =0;
    while (startAddr != 0){
        indexblk[indexsize] = readBlockFromDisk(startAddr,buf);
        startAddr = getNxtAddr(indexblk[indexsize]);
        indexsize++;
    }
    unsigned char *blk;


    int firstblk = 0;//需要遍历的第一个目录项
    int i;//需要遍历的最后一个目录项
    for (i = 0;i<indexsize*7;i++) {
        int X1 = getX(indexblk[i / 7], i % 7);
        if (X1 == 0 || X1 > X){
            if (i != 0) i--;
            break;
        }
        if (X1 < X){
            firstblk = i;
            continue;
        }
    }

    int current;
    while (firstblk <= i){
        blk = readBlockFromDisk(getY(indexblk[firstblk/7],firstblk%7),buf);
        for (int j=0;j<7;j++){
            if (getX(blk,j)==X){
                int X1 = X;
                int Y1 = Y;
                int X2 = getX(blk,j);
                int Y2 = getY(blk,j);
                //printf("find %d,%d\n", getX(blk,j),getY(blk,j));
                current =  writeoutput(X2, Y2,outputblk);
                if (current == BLK_FULL) {
                    writeBlockToDisk(outputblk, maxUsedAddr, buf);
                    outputblk = getNewBlockInBuffer(buf);
                }
            }
        }
        freeBlockInBuffer(blk,buf);
        firstblk++;
    }

    outputEnd(outputblk,current,buf);
    freeBlockInBuffer(outputblk,buf);
    for (i = 0;i<5;i++) {
        freeBlockInBuffer(indexblk[i], buf);
    }

}
void innerjoin(Buffer *buf){
    unsigned char *blk4R;
    unsigned char *outputblk = getNewBlockInBuffer(buf);
    unsigned char *indexblk[5];
    int indexsize =0;
    int startAddr = 148;
    while (startAddr != 0){
        indexblk[indexsize] = readBlockFromDisk(startAddr,buf);
        startAddr = getNxtAddr(indexblk[indexsize]);
        indexsize++;
    }
    unsigned char *blk;
    int current;

    for (int i=1;i<=16;i++){
        blk4R = readBlockFromDisk(i,buf);
        for (int p=0;p<7;p++){
            int X = getX(blk4R,p);
            int Y = getY(blk4R,p);
            //selectInIndex(148, getX(blk4R,p), getY(blk4R,p),buf,join);
            int firstblk = 0;//需要遍历的第一个目录项
            int i;//需要遍历的最后一个目录项
            for (i = 0;i<indexsize*7;i++) {
                int X1 = getX(indexblk[i / 7], i % 7);
                if (X1 == 0 || X1 > X){
                    if (i != 0) i--;
                    break;
                }
                if (X1 < X){
                    firstblk = i;
                    continue;
                }
            }

            while (firstblk <= i){
                blk = readBlockFromDisk(getY(indexblk[firstblk/7],firstblk%7),buf);
                for (int j=0;j<7;j++){
                    if (getX(blk,j)==X){
                        int X1 = X;
                        int Y1 = Y;
                        int X2 = getX(blk,j);
                        int Y2 = getY(blk,j);
                        //printf("find %d,%d\n", getX(blk,j),getY(blk,j));
                        current = writeoutput(X1, Y1,outputblk);
                        if (current == BLK_FULL) {
                            writeBlockToDisk(outputblk, maxUsedAddr, buf);
                            outputblk = getNewBlockInBuffer(buf);
                        }
                        current =  writeoutput(X2, Y2,outputblk);
                        if (current == BLK_FULL) {
                            writeBlockToDisk(outputblk, maxUsedAddr, buf);
                            outputblk = getNewBlockInBuffer(buf);
                        }
                    }
                }
                freeBlockInBuffer(blk,buf);
                firstblk++;
            }
        }
        freeBlockInBuffer(blk4R,buf);
    }

    outputEnd(outputblk,current,buf);
    freeBlockInBuffer(outputblk,buf);
    for (int i = 0;i<5;i++) {
        freeBlockInBuffer(indexblk[i], buf);
    }
}
void and(Buffer *buf){
    unsigned char *blk4R;
    unsigned char *outputblk = getNewBlockInBuffer(buf);
    unsigned char *indexblk[5];
    int indexsize =0;
    int startAddr = 148;
    while (startAddr != 0){
        indexblk[indexsize] = readBlockFromDisk(startAddr,buf);
        startAddr = getNxtAddr(indexblk[indexsize]);
        indexsize++;
    }
    unsigned char *blk;
    int current;

    for (int i=1;i<=16;i++){
        blk4R = readBlockFromDisk(i,buf);
        for (int p=0;p<7;p++){
            int X = getX(blk4R,p);
            int Y = getY(blk4R,p);
            //selectInIndex(148, getX(blk4R,p), getY(blk4R,p),buf,join);
            int firstblk = 0;//需要遍历的第一个目录项
            int i;//需要遍历的最后一个目录项
            for (i = 0;i<indexsize*7;i++) {
                int X1 = getX(indexblk[i / 7], i % 7);
                if (X1 == 0 || X1 > X){
                    if (i != 0) i--;
                    break;
                }
                if (X1 < X){
                    firstblk = i;
                    continue;
                }
            }

            int flag = 0;//是否有相同元组
            while (firstblk <= i){
                blk = readBlockFromDisk(getY(indexblk[firstblk/7],firstblk%7),buf);
                for (int j=0;j<7;j++){
                    if (getX(blk,j)==X && getY(blk,j)==Y){
                        flag = 1;
                    }
                }
                freeBlockInBuffer(blk,buf);
                firstblk++;
            }

            if (flag == 0){//如果没有相同就把R中元组写入
                current = writeoutput(X, Y,outputblk);
                if (current == BLK_FULL) {
                    writeBlockToDisk(outputblk, maxUsedAddr, buf);
                    outputblk = getNewBlockInBuffer(buf);
                }
            }

        }
        freeBlockInBuffer(blk4R,buf);
    }


    for (int i=17;i<=48;i++){//然后把S全部写入
        blk4R = readBlockFromDisk(i,buf);
        for (int j =0;j<7;j++){
            current = writeoutput(getX(blk4R,j), getY(blk4R,j),outputblk);
            if (current == BLK_FULL) {
                writeBlockToDisk(outputblk, maxUsedAddr, buf);
                outputblk = getNewBlockInBuffer(buf);
            }
        }
        freeBlockInBuffer(blk4R,buf);
    }


    outputEnd(outputblk,current,buf);
    freeBlockInBuffer(outputblk,buf);
    for (int i = 0;i<5;i++) {
        freeBlockInBuffer(indexblk[i], buf);
    }
}

//升序
void sortblk(unsigned char *blk, Buffer *buf){
    int p = 6;
    while (getX(blk,p)==0)p--;
    if (p==0) return;
    for (int i = 0;i<=p;i++){
        int min = 9999;
        int k=i;
        for (int j = i;j<=p;j++){
            if (getX(blk,j) < min){
                min = getX(blk,j);
                k = j;
            }
        }
        int X1 = getX(blk,k);
        int Y1 = getY(blk,k);
        int X2 = getX(blk,i);
        int Y2 = getY(blk,i);
        setX(blk,k,X2);
        setX(blk,i,X1);
        setY(blk,k,Y2);
        setY(blk,i,Y1);
    }
}