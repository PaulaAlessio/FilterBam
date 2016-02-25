//CreateTreeFromFilter.cc 
#include "CreateTreeFromFilter.h"
#define K 12 //We construct a K-ary tree. 12 to allow for termination
int LT[256];
/*****************************/
/*look up table*/
void InitMap(){
   for (int i=0 ; i<256;i++){
      LT[i] = i-'0';
   }
}

/*****************************/
//Functions with nodes
tree CreateTree(){
      tree newnode = new(struct node);
      newnode->val = 0;
      for (int i=0;i<K;i++){
         newnode->children[i]=NULL;
      }
      return newnode;
}


void InsertChain(tree &arbol,int *cadena,int length){
    tree current = arbol;
    for (int i=0; i<length;i++){
          if (current->children[cadena[i]]==NULL){ 
              current->children[cadena[i]] = CreateTree();
          }
          current = current->children[cadena[i]];
          current->val++;
   }
}

bool CheckPath(tree &arbol,int *cadena,int length){
    tree current = arbol;
    for (int i=0; i<length;i++){
       if(cadena[i]>=0 && cadena[i]<K){
          if (current->children[cadena[i]]==NULL){ 
             return false;
           }
          else 
            current = current->children[cadena[i]];
       }
       else {
          return false;
       }
    }
    return true;
}


/*****************************/
// Create TreeFromFilter
void CreateTreeFromFilter(tree &arbol,const std::string &filter_file,std::string &prefix){
   
   size_t pos_start = prefix.length();
   FILE *fil_in;
   fil_in = fopen(filter_file.c_str(),"r");
   if (fil_in==NULL){
      std::cerr << "Filter file " << filter_file << "not found" << std::endl;
      throw std::runtime_error(std::strerror(errno));
   }
   char buffer[B_LEN];
   size_t bend; // buffer end 
   size_t t_size=T_DEPTH;
   char s[T_DEPTH]; // string for the buffering
   s[0] = '\0';
   char * ptr;
   size_t pos_now = 0, idx = 0, tab = 0;
   int node_values[t_size];
   int lines=0;
   InitMap();
   // Read the file and construct the tree on the fly 
   while (!feof(fil_in)){
      bend = fread(buffer,1,B_LEN,fil_in);
      ptr = buffer;
      while (bend--){
         if (*ptr == '\n'){
            lines++;
            tab = 0;
            pos_now = -1;
            for (size_t i=0; i<idx;i++){
                node_values[i] = LT[s[i]];
            }
            node_values[idx++]=11;
            InsertChain(arbol,node_values,idx);
            idx = 0;
            s[0]='\0';
         }
         else if(*ptr == '\t'){
            tab++;
         }
         else if(pos_now >= pos_start && tab == 0){
           s[idx] = *ptr;  
           idx++; 
         }
         ptr++;   
         pos_now++; 
      }
   }
   fclose(fil_in);
   std::cerr << "Position in line: " << idx << " Field: " << tab << " Lines " << lines << std::endl ; 
   if(tab){
      std::cerr << "soy la ultima linea" << std::endl;
      for(size_t i=0;i<idx;i++){
            node_values[i] = LT[s[i]];
      }
      InsertChain(arbol,node_values,idx);
   }
}


