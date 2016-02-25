//CreateTreeFromFilter.h 
#ifndef CREATETREEFROMFILTER_H
#define  CREATETREEFROMFILTER_H

#include <cstdio>
#include <stdexcept>
#include <iostream>
#include <string>
#include <cstring>

#define B_LEN 65536
#define T_DEPTH 32

extern int LT[256];

struct node{
   int val; 
   node * children[11];
};
typedef struct node *tree;

void CreateTreeFromFilter(tree &arbol,const std::string &filter_file,std::string &prefix);
void InitMap();
tree CreateTree();
void InsertChain(tree &arbol,int *cadena,int length=32);
bool CheckPath(tree &arbol,int *cadena,int length=32);


#endif
