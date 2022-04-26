#include "Query.h"

Query::Query(int type) : type(type)
{
    glGenQueries(1, &id);
}

Query::~Query()
{
    glDeleteQueries(1, &id);
}

void Query::start()
{
    glBeginQuery(type, id);
    inUse = true;
}

void Query::end()
{
    glEndQuery(type);
}

bool Query::isResultReady()
{
    int res;
    glGetQueryObjectiv(id, GL_QUERY_RESULT_AVAILABLE, &res);
    return res == GL_TRUE;
}

bool Query::isInUse()
{
    return inUse;
}

int Query::getResult()
{
    int res;
    inUse = false;
    glGetQueryObjectiv(id, GL_QUERY_RESULT, &res);
    return res;
}
