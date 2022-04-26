#ifndef QUERY_H_
#define QUERY_H_

#include <GL/glew.h>

/* Class based on ThinMatrix's youtube tutorial on occlusion queries  */
// TODO: lien

class Query {

    public:
        /**
         * Constructor : initialize the openGL object
         *  - type (int) : the type of query to perform
         */
        Query(int type);
        // destructor (destroys the openGL object)
        ~Query();

        /**
         *  Starts the query
         */
        void start();

        /**
         *  Stops the query
         */
        void end();

        /**
         *  Returns true if the query is finished
         */
        bool isResultReady();

        /**
         *  Return true if the result has been got with getResult()
         */
        bool isInUse();

        /**
         *  Returns the result of the query
         */
        int  getResult();

    private:
        bool inUse = false;
        GLuint id;
        const int type;
};

#endif // QUERY_H_
