#include "CDatabase.h"

#include <q3http.h>
#include <q3urloperator.h>
#include <qstring.h>
#include <q3network.h>
#include <q3networkprotocol.h>
//#include <qbytearray.h>
#include <qapplication.h>
#include <qfile.h>
//Added by qt3to4:
#include <Q3CString>

#include <sstream>
#include <iomanip>

int mystrcasecmp( const char *s1, const char *s2, int n)
{// this function compares the two strings upto n characters
// the case of the characters is not taken into account a == A
// and then returns a zero if they are equal otherwise
// none zero value

  for (int i=0;i < n;i++)
  {
    char ch1 = toupper( s1[ i ] );
    char ch2 = toupper( s2[ i ] );

    if ( ch1 != ch2 )
    {
      return (int) (ch1 - ch2);
    }
  }

  return (0);
}

// the object which allows the progam to communicate with the PDB
CDatabaseCom::CDatabaseCom(std::string user, std::string password, std::string SN){

  m_SN = SN;
  m_qhttp = new Q3Http(NULL,"PWE_COM");
  m_qhttp->setHost("wacap.unige.ch",3146);

  std::string tobe_encoded = user + ":" + password;
  Q3CString result = KCodecs::base64Encode(Q3CString(tobe_encoded.c_str()));

  m_encoded_password = (const char*)result;

  QObject::connect(m_qhttp, SIGNAL(requestStarted(int)),(QObject*)this,SLOT(Request_Started(int)));
  QObject::connect(m_qhttp, SIGNAL(requestFinished(int,bool)),(QObject*)this,SLOT(Read_Incoming(int,bool)));

  m_complete = true;

}


CDatabaseCom::~CDatabaseCom(){// the destructor
}

void CDatabaseCom::Request_Started(int){// this function is called when the request is commenced
  m_htmlfile.clear();
  m_complete = false;
}

void CDatabaseCom::Read_Incoming(int , bool ){
// this function is connected to the signal
// that is sent when there is data to be read

  int i;
  std::string error_str,html_line="";
  QByteArray data;

  Q3Http::Error val = m_qhttp->error();

  switch(val){
  case (Q3Http::NoError):{
    data = m_qhttp->readAll();
    error_str = "No error occurred"; 
    
    for (i=0;i<(int)data.count();i++){
      html_line += data[i];
      if (data[i] == '\n'){
	m_htmlfile.push_back(html_line);
	//printf("%s",html_line.c_str());
	html_line = "";
      }
    }
    if(m_htmlfile.size()>0 && ( (int)(*(m_htmlfile.end()-1)).find("</HTML>")!=(int)std::string::npos
				|| (int)(*(m_htmlfile.end()-1)).find("</html>")!=(int)std::string::npos)){
      emit DownloadComplete(error_str.c_str());
      m_complete = true;
    }
    return;}

  case (Q3Http::HostNotFound): error_str = "The host name lookup failed";break;
  case (Q3Http::ConnectionRefused): error_str = "The server refused the connection";break;
  case (Q3Http::UnexpectedClose): error_str ="The server closed the connection unexpectedly";break;
  case (Q3Http::InvalidResponseHeader): error_str ="The server sent an invalid response header";break;
  case (Q3Http::WrongContentLength): error_str ="The client could not read the content correctly because an error with respect to the content length occurred";break;
  case (Q3Http::Aborted): error_str ="The request was aborted with abort()";break;
  default:
  case (Q3Http::UnknownError): error_str ="An error other than those specified above occurred";break;
  }
  
  emit DownloadComplete(error_str.c_str());
  m_complete = true;

}


void CDatabaseCom::Write_Socket(std::string command){
// the user has press the button and we will send the query
  Q3HttpRequestHeader http_header(command.c_str());
  m_id = m_qhttp->request(http_header);
}

std::vector<std::string> CDatabaseCom::getStaveSNs(QApplication &app){
  std::vector<std::string> retvec;
  m_complete = false;
  std::string checkStr = m_SN;
  checkStr.erase(11,14);
  if(checkStr=="20212000018" || checkStr=="20212001120") // get sectors for disk or bistaves for half-shell
    GetHtmlAss(false);
  else
    GetHtml();
  while(!m_complete )
    app.processEvents();
//   QString inError = error_msg;
//   if("No error occurred"!=inError){
//     QApplication::restoreOverrideCursor();
//     QMessageBox::warning(this,"tmperr","Error in html reading:\n"+inError);
//     return;
//   }
//   if(m_cdb->m_htmlfile.size()==0){
//     QApplication::restoreOverrideCursor();
//     QMessageBox::warning(this,"tmperr","Error in html reading:\nno data returned");
//     return;
//   }
  int pos;
  for(unsigned int i=0;i<m_htmlfile.size();i++){
    //    printf("%s\n", m_htmlfile[i].c_str());
    // modules
    if((pos=m_htmlfile[i].find("Z_CHK=0&P_SER_NO=20210020"))!=(int)std::string::npos){
      m_htmlfile[i].erase(0,pos+25);
      m_htmlfile[i].erase(6,m_htmlfile[i].length()-6);
      retvec.push_back(m_htmlfile[i]);
    }
    // sectors/staves
    if((int)m_htmlfile[i].find(checkStr)==(int)std::string::npos && 
       ((pos=m_htmlfile[i].find("Z_CHK=0&P_SER_NO=2021200001"))!=(int)std::string::npos ||
	(pos=m_htmlfile[i].find("Z_CHK=0&P_SER_NO=2021200111"))!=(int)std::string::npos)){
      m_htmlfile[i].erase(0,pos+24);
      m_htmlfile[i].erase(7,m_htmlfile[i].length()-7);
      retvec.push_back(m_htmlfile[i]);
    }
    // bi-staves
    if((int)m_htmlfile[i].find(checkStr)==(int)std::string::npos && 
       ((pos=m_htmlfile[i].find("Z_CHK=0&P_SER_NO=2021200002"))!=(int)std::string::npos ||
	(pos=m_htmlfile[i].find("Z_CHK=0&P_SER_NO=2021200112"))!=(int)std::string::npos)){
      m_htmlfile[i].erase(0,pos+24);
      m_htmlfile[i].erase(7,m_htmlfile[i].length()-7);
      retvec.push_back(m_htmlfile[i]);
    }
  }
  return retvec;
}
void CDatabaseCom::getParent(QApplication &app, std::string &parentSN, int &assPos)
{
  std::string orgSN = m_SN;

  // first, find parent item

  m_complete = false;
  GetHtmlAss();
  while(!m_complete )
    app.processEvents();
  parentSN = "";
  assPos = -1;
  for(unsigned int i=0;i<m_htmlfile.size();i++){
    int pos;
    if((pos=m_htmlfile[i].find("Z_CHK=0&P_SER_NO=2021"))!=(int)std::string::npos){
      m_htmlfile[i].erase(0,pos+17);
      m_htmlfile[i].erase(14,m_htmlfile[i].length()-14);
      parentSN = m_htmlfile[i];
      break;
    }
  }
  m_htmlfile.clear();
  if(parentSN=="") return;

  // then get list of object that [arent is made of, find object type and its position

  m_SN = parentSN;
  m_complete = false;
  GetHtml();
  while(!m_complete )
    app.processEvents();
  int apos = 0;
  std::string searchp;
  for(unsigned int j=0;j<m_htmlfile.size();j++){
    int pos;
    searchp = "h_assm_no=";
    searchp += orgSN;
    if(((int)m_htmlfile[j].find(searchp))!=(int)std::string::npos){
      searchp = m_htmlfile[j];
      pos = searchp.find("\"_blank\">");
      searchp.erase(0,pos+9);
      pos = searchp.find("</a>");
      searchp.erase(pos,searchp.length()-pos);
      //printf("object is of type %s\n",searchp.c_str());
      break;
    }
  }  
  for(unsigned int j=0;j<m_htmlfile.size();j++){
    int pos;
    if(((int)m_htmlfile[j].find(searchp))!=(int)std::string::npos){
      pos = m_htmlfile[j].find("h_assm_no=");
      m_htmlfile[j].erase(0,pos+10);
      m_htmlfile[j].erase(14,m_htmlfile[j].length()-14);
      //printf("found %s at %d\n",m_htmlfile[j].c_str(), apos);
      if(m_htmlfile[j] == orgSN){
 	assPos = apos;
	break;
      }
      apos++;
    }
  }  

  m_SN = orgSN;

  return;

}
void CDatabaseCom::GetHtml(){
// this function will get the html file from the server

  std::string command;
  std::string post_data;
  
  post_data = "h_assm_no_from=" + m_SN + "&h_assm_no_to=" + m_SN + 
    "&h_comp_no_from=&h_comp_no_to=&h_assm_descr=&h_item_descr=&h_locn_name_item=&h_owner_assm="+
    "&h_assm_date_o=01+jan+1999&h_assm_date_n=today";
  std::stringstream length_str;
  length_str << "Content-length: " << post_data.length() << std::endl;

  std::string content_length;
  content_length = "Content-type: text/html\n";
  content_length += length_str.str() + "\n\n";
  command = "GET /pls/lopaca/PIX_R_ASSM.BASIC_REPORT?"+ post_data + " HTTP/1.1\n";
  command += "Host: www.fisica.uniud.it\n";
  command += "Authorization: Basic " + m_encoded_password + "\n\n";


  Write_Socket(command);
  return;
}
void CDatabaseCom::GetHtmlAss(bool parent){
// this function will get the html file from the server

  std::string command;
  std::string post_data;
  
  if(parent)
    post_data = "h_assm_no=&h_comp_no=" + m_SN;
  else
    post_data = "h_assm_no=" + m_SN+"&h_comp_no=";
  std::stringstream length_str;
  length_str << "Content-length: " << post_data.length() << std::endl;

  std::string content_length;
  content_length = "Content-type: text/html\n";
  content_length += length_str.str() + "\n\n";
  command = "GET /pls/lopaca/PIX_R_ASSM_LIGHT.BASIC_REPORT?"+ post_data + " HTTP/1.1\n";
  command += "Host: www.fisica.uniud.it\n";
  command += "Authorization: Basic " + m_encoded_password + "\n\n";

  Write_Socket(command);
  return;
}


// this is a name class that is used for
// all the functions necessary to 
//
const char KCodecs::Base64EncMap[64] =
 {
   0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
   0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50,
   0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
   0x59, 0x5A, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
   0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E,
   0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,
   0x77, 0x78, 0x79, 0x7A, 0x30, 0x31, 0x32, 0x33,
   0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2B, 0x2F
 };
 
 const char KCodecs::Base64DecMap[128] =
 {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x3F,
   0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
   0x3C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
   0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
   0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
   0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
   0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
   0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
   0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00
 };




 Q3CString KCodecs::base64Encode( const Q3CString& str, bool insertLFs )
 {
     if ( str.isEmpty() )
         return "";
 
     QByteArray in (str.length());
     memcpy( in.data(), str.data(), str.length() );
     return base64Encode( in, insertLFs );
 }
 
 Q3CString KCodecs::base64Encode( const QByteArray& in, bool insertLFs )
 {
     QByteArray out;
     base64Encode( in, out, insertLFs );
     return Q3CString( out.data(), out.size()+1 );
 }
 
 void KCodecs::base64Encode( const QByteArray& in, QByteArray& out,
                             bool insertLFs )
 {
     // clear out the output buffer
     out.resize (0);
     if ( in.isEmpty() )
         return;

     int sidx = 0;
     int didx = 0;
     
//     unsigned int sidx = 0;
//     unsigned int didx = 0;
     const char* data = in.data();
     const unsigned int len = in.size();
 
     unsigned int out_len = ((len+2)/3)*4;
 
     // Deal with the 76 characters or less per
     // line limit specified in RFC 2045 on a
     // pre request basis.
     insertLFs = (insertLFs && out_len > 76);
     if ( insertLFs )
       out_len += ((out_len-1)/76);
 
     int count = 0;
     out.resize( out_len );
 
 
     out[didx] = '\n';

     // 3-byte to 4-byte conversion + 0-63 to ascii printable conversion
     if ( len > 1 )
     {
         while (sidx < (int)len-2)
         {
             if ( insertLFs )
             {
                 if ( count && (count%76) == 0 )
                 {                   
                  out[didx++] = '\n';

                 }
                 count += 4;
            }
             out[didx++] = Base64EncMap[(data[sidx] >> 2) & 077];
             out[didx++] = Base64EncMap[((data[sidx+1] >> 4) & 017) |
                                        ((data[sidx] << 4) & 077)];
             out[didx++] = Base64EncMap[((data[sidx+2] >> 6) & 003) |
                                        ((data[sidx+1] << 2) & 077)];
             out[didx++] = Base64EncMap[data[sidx+2] & 077];
             sidx += 3;
         }
     }
 
     if (sidx < (int)len)
     {
         if ( insertLFs && (count > 0) && (count%76) == 0 )
            out[didx++] = '\n';
 
         out[didx++] = Base64EncMap[(data[sidx] >> 2) & 077];
         if (sidx < (int)len-1)
         {
	   out[didx++] = Base64EncMap[((data[sidx+1] >> 4) & 017) |
				      ((data[sidx] << 4) & 077)];
             out[didx++] = Base64EncMap[(data[sidx+1] << 2) & 077];
         }
         else
         {
             out[didx++] = Base64EncMap[(data[sidx] << 4) & 077];
         }
     }
 
     // Add padding
     while (didx < (int)out.size())
     {
         out[didx] = '=';
         didx++;
     }
 }
 
 Q3CString KBase64::base64Decode( const Q3CString& str )
 {
     if ( str.isEmpty() )
         return "";
 
     QByteArray in( str.length() );
     memcpy( in.data(), str.data(), str.length() );
     return base64Decode( in );
 }
 
 Q3CString KBase64::base64Decode( const QByteArray& in )
 {
     QByteArray out;
     base64Decode( in, out );
     return Q3CString( out.data(), out.size()+1 );
 }

 void KBase64::base64Decode( const QByteArray& in, QByteArray& out )
{
     out.resize(0);
    if ( in.isEmpty() )
         return;
 
     unsigned int count = 0;
     unsigned int len = in.size(), tail = len;
     const char* data = in.data();
 
     // Deal with possible *nix "BEGIN" marker!!
     while ( count < len && (data[count] == '\n' || data[count] == '\r' ||
             data[count] == '\t' || data[count] == ' ') )
         count++;

//    int mystrcasecmp( const char *s1, const char *s2, int n) 
//     if ( strncasecmp(data+count, "begin", 5) == 0 )
     if ( mystrcasecmp(data+count, "begin", 5) == 0 )
     {
         count += 5;
         while ( count < len && data[count] != '\n' && data[count] != '\r' )
             count++;
 
         while ( count < len && (data[count] == '\n' || data[count] == '\r') )
             count ++;
 
         data += count;
         tail = (len -= count);
     }
 
     // Find the tail end of the actual encoded data even if
     // there is/are trailing CR and/or LF.
     while ( data[tail-1] == '=' || data[tail-1] == '\n' ||
             data[tail-1] == '\r' )
         if ( data[--tail] != '=' ) len = tail;
 
     int outIdx = 0;
//     unsigned int outIdx = 0;
     out.resize( (count=len) );
//     for (unsigned int idx = 0; idx < count; idx++)
     for (int idx = 0; idx < (int)count; idx++)
     {
         // Adhere to RFC 2045 and ignore characters
         // that are not part of the encoding table.
         char ch = data[idx];
         if ((ch > 47 && ch < 58) || (ch > 64 && ch < 91) ||
             (ch > 96 && ch < 123) || ch == '+' || ch == '/' || ch == '=')
         {
	   out[outIdx++] = (char)Base64DecMap[(int)ch];
         }
         else
         {
             len--;
             tail--;
         }
     }
 
     // kdDebug() << "Tail size = " << tail << ", Length size = " << len << endl;
 
     // 4-byte to 3-byte conversion
     len = (tail>(len/4)) ? tail-(len/4) : 0;
//     unsigned int sidx = 0, didx = 0;
     int sidx = 0, didx = 0;
     if ( len > 1 )
     {
       while (didx < (int)len-2)
       {
           out[didx] = (((out[sidx] << 2) & 255) | ((out[sidx+1] >> 4) & 003));
           out[didx+1] = (((out[sidx+1] << 4) & 255) | ((out[sidx+2] >> 2) & 017));
           out[didx+2] = (((out[sidx+2] << 6) & 255) | (out[sidx+3] & 077));
           sidx += 4;
           didx += 3;
       }
     }

     if (didx < (int)len)
         out[didx] = (((out[sidx] << 2) & 255) | ((out[sidx+1] >> 4) & 003));
 
     if (++didx < (int)len )
         out[didx] = (((out[sidx+1] << 4) & 255) | ((out[sidx+2] >> 2) & 017));
 
     // Resize the output buffer
     if ( len == 0 || (int)len < (int)out.size() )
       out.resize(len);
 }
 
RDBManager::RDBManager(QApplication *app) : m_app(app){
  m_html_line = "";
  m_qhttp = 0;
  m_fout  = 0;
}
RDBManager::~RDBManager(){
}

std::vector<std::string> RDBManager::getRootUrls(QString modSN, QString measType){
  std::vector<std::string> retvec;
  QString searchString = modSN+"/"+measType;
  QString host = "atlas1.ge.infn.it";
  QString url = "/pixelprod/ListFiles2.php?Site=ALL&Item="+searchString;
  QString rname, totalRname, path_line, old_path="nn";
  getRawData(url,host);
  // wait for request to finish
  while(m_qhttp!=0){
    if(m_app!=0) m_app->processEvents();
  }
  int pos=-1, pos2=-1, offs=0, pos3=-1;
  QString html_line = m_html_line;
  pos3 = html_line.find(searchString+"/");
  while(pos3>=0){
    path_line = html_line;
    path_line.remove(0,pos3);
    path_line.remove(searchString.length()+3, path_line.length()-(searchString.length()+3));
    if(path_line!=old_path){
      url = "/pixelprod/ListFiles2.php?Site=ALL&Item=" + path_line;
      m_html_line="";
      getRawData(url,host);
      // wait for request to finish
      while(m_qhttp!=0){
	if(m_app!=0) m_app->processEvents();
      }
      QString tmp_url = m_html_line;
      pos = tmp_url.find(".root");
      while(pos>=0){
	for(offs=10;offs<pos;offs+=10){
	  pos2 = tmp_url.find("a href=",(pos-offs));
	  if(pos2<pos && pos2>=0) break;
	}
	if(pos2<pos && pos2>=0){
	  tmp_url = tmp_url.mid(pos2+8,(pos-pos2-3));
	  tmp_url.remove(0,7);
	  retvec.push_back(tmp_url.latin1());
	}
	tmp_url = m_html_line;
	pos = tmp_url.find(".root",pos+100);
      }
    }
    old_path = path_line;
    pos3 = html_line.find(searchString+"/",pos3+searchString.length()+2);
  }
  return retvec;
}
int RDBManager::getRootFile(QString modSN, QString measType, QString outPath, bool retainName){
  QString searchString = modSN+"/"+measType;
  QString host = "atlas1.ge.infn.it";
  QString url = "/pixelprod/ListFiles2.php?Site=ALL&Item="+searchString;
  QString rname, totalRname;
  getRawData(url,host);
  // wait for request to finish
  while(m_qhttp!=0){
    if(m_app!=0) m_app->processEvents();
  }
  int pos=-1, pos2=-1, offs=0;
  pos = m_html_line.find(searchString+"/");
  if(pos>=0){
    m_html_line.remove(0,pos);
    m_html_line.remove(searchString.length()+3, m_html_line.length()-(searchString.length()+3));
    searchString = m_html_line;
    url = "/pixelprod/ListFiles2.php?Site=ALL&Item="+searchString;
    getRawData(url,host);
    // wait for request to finish
    while(m_qhttp!=0){
      if(m_app!=0) m_app->processEvents();
    }
    pos = m_html_line.find(".root");
    if(pos>=0){
      for(offs=10;offs<pos;offs+=10){
	pos2 = m_html_line.find("a href=",(pos-offs));
	if(pos2<pos && pos2>=0) break;
      }
      if(pos2<pos && pos2>=0){
	m_html_line = m_html_line.mid(pos2+8,(pos-pos2-3));
	m_html_line.remove(0,7);
	pos = m_html_line.find("/");
	if(pos>=0){
	  host = m_html_line.left(pos);
	  url  = m_html_line.right(m_html_line.length()-pos);
	  if(retainName){
	    rname = url;
	    pos = rname.find("/");
	    pos2 = pos;
	    while(pos>=0){
	      pos2 = pos;
	      pos = rname.find("/",pos+1);
	    }
	    rname.remove(0,pos2+1);
	    //	    printf("host: %s\nURL: %s\nFilename: %s\n",host.latin1(),url.latin1(),rname.latin1());
	    rname = outPath+"/"+rname;
	  } else
	    rname = outPath;
	  getRawData(url,host,rname);
	  // wait for request to finish
	  while(m_qhttp!=0){
	    if(m_app!=0) m_app->processEvents();
	  }
	  return 0;
	} else
	  return -4;
      } else
	return -3;
    } else
      return -2;
  } else
    return -1;
}
void RDBManager::getRootFromUrl(QString input_url, QString outPath){
  QString host, url;
  int pos = input_url.find("/");
  if(pos>=0){
    host = input_url.left(pos);
    url  = input_url.right(input_url.length()-pos);
    getRawData(url,host,outPath);
    // wait for request to finish
    while(m_qhttp!=0){
      if(m_app!=0) m_app->processEvents();
    }
  }
  return;
}
void RDBManager::getRawData(QString url, QString host, QString path){
  delete m_qhttp;
  m_qhttp = new Q3Http(NULL,"RDB_COM");
  m_qhttp->setHost(host);

  QObject::connect(m_qhttp, SIGNAL(requestFinished(int,bool)),(QObject*)this,SLOT(readRawData()));
  QObject::connect(m_qhttp, SIGNAL(done(bool)),(QObject*)this,SLOT(procRawData()));

  // let's send the request
  m_html_line="";
  if(path!=QString::null){
    m_fout = new QFile(path);
    m_fout->open(QIODevice::WriteOnly);
  }
  m_qhttp->get(url,m_fout);
}
void RDBManager::readRawData(){
  Q3Http::Error val = m_qhttp->error();
  QString error_str;
  QByteArray data;

  switch(val){
  case (Q3Http::NoError):{
    if(m_fout==0){
      data = m_qhttp->readAll();
      for (int i=0;i<(int)data.count();i++){
	m_html_line += data[i];
      }
    }
    return ;
  }
  case (Q3Http::HostNotFound): error_str = "The host name lookup failed";break;
  case (Q3Http::ConnectionRefused): error_str = "The server refused the connection";break;
  case (Q3Http::UnexpectedClose): error_str ="The server closed the connection unexpectedly";break;
  case (Q3Http::InvalidResponseHeader): error_str ="The server sent an invalid response header";break;
  case (Q3Http::WrongContentLength): error_str ="The client could not read the content correctly because an error with respect to the content length occurred";break;
  case (Q3Http::Aborted): error_str ="The request was aborted with abort()";break;
  default:
  case (Q3Http::UnknownError): error_str ="An unknown error occurred";break;
  }
}
void RDBManager::procRawData(){
  delete m_qhttp;
  m_qhttp = 0;
  delete m_fout;
  m_fout = 0;
}
