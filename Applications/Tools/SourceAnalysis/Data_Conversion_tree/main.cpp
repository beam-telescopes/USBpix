#include <string>
#include <string.h>

void cluster(std::string intag, int initial, int final);
void raw_to_tree(std::string intag, int initial, int final);

int main(int argc, char* argv[]) {
  //cout << "argc = " << argc << endl;
  std::string intag;     //tag of data
  int initial;      //initial index
  int final;        //final index
  bool runConv=false;
  bool runClus=false;

  for(int i = 0; i < argc; i++){

    
    if (strcmp(argv[i],"--tag") == 0){
      intag=argv[i+1];
      //cout << "input data tag =" << intag <<endl;
    }
    
    if (strcmp(argv[i],"-i") == 0){
      initial=atoi(argv[i+1]);
      //cout << "initial =" << initial <<endl;
    }
    
    if (strcmp(argv[i],"-f") == 0){
      final=atoi(argv[i+1]);
      //cout << "final =" << final <<endl;
    }
    
    if (strcmp(argv[i],"--conv") == 0){
      runConv=true;
    }
    if (strcmp(argv[i],"--clus") == 0){
      runClus=true;
    }
    if (strcmp(argv[i],"--all") == 0){
      runConv=true;
      runClus=true;
    }
  }

  if(runConv) raw_to_tree(intag, initial, final);
  if(runClus) cluster(intag, initial, final);
  
}
