#include "pistache/endpoint.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "fasttext.h"
#include <string>
#include <sstream>

using namespace Net;
using namespace rapidjson;
using namespace fasttext;

FastText ft;

class DataHandler : public Http::Handler {
public:

    HTTP_PROTOTYPE(DataHandler)

    void onRequest(const Http::Request& request, Http::ResponseWriter response) {
		Document doc;
                doc.Parse(request.body().c_str());
                assert(doc.HasMember("data"));
		Value& lines = doc["data"];
		
		StringBuffer s;
	        Writer<StringBuffer> writer(s);	
		writer.StartObject();
		writer.Key("results");
		writer.StartArray();

		for(int i=0; i < lines.Size(); i++)
		{
			std::vector<std::pair<real,std::string>> predictions;
  			std::istringstream in_string(lines[i].GetString());
			ft.predict(in_string, 1, predictions);
  			if (predictions.empty()) 
			{
    				std::cout << "n/a" << std::endl;
				writer.String("None");
  			}
  
			for (auto it = predictions.cbegin(); it != predictions.cend(); it++)
			{ 
    				std::cout << it->second;
				writer.String((it->second).c_str());
			}
		}
		writer.EndArray();
		writer.EndObject();
		response.send(Http::Code::Ok, s.GetString(), MIME(Application, Json));
    }
};

int main(int argc, char **argv) {
    	utils::initTables();

        if(argc != 2)
        {
          std::cout << "Please provide model" << std::endl;
          exit(EXIT_FAILURE);
        }

	std::cout << "Loading model : " << argv[1] << std::endl;
	ft.loadModel(std::string(argv[1]));

	Http::listenAndServe<DataHandler>("*:9080");
	
	utils::freeTables();
	exit(0);
}

