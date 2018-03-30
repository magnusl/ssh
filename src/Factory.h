/* File:            Factory
 * Description:     The Factory class allows objects to created under runtime
 *                  using a supplied key.
 * Author:          Magnus Leksell
 *****************************************************************************/
#ifndef _FACTORY_H_
#define _FACTORY_H_

#include <map>

template <class T, class Key class CreateParam>
class Factory
{
public: 

    typedef T* (* CreateFunc) (const CreateParam &);

    /* Function:        registerInstance
     * Description:     Registers a factory function with a name. The "param" value is supplied
     *                  to the factory function when the factory function is called. This allows Multiple 
     *                  registered instances to share the same factory function.
     */
    bool registerInstance(Key & key, CreateFunc func, CreateParam param)
    {
        map[key] = std::pair<func, param>;
    }

    /* create the instance of the param */
    T * createInstance(Key & name)
    {
        std::map<Key, CreateFunc, CreateParam>::iterator it =
            std::objects.find(name);

        if(it == objects.end())
            return 0;
        else
        {
            CreateFunc * create_function    = it.second.first;
            CreateParam param               = it.second.second;
            if(create_function == NULL)
                return 0;
            // now let the supplied function create the algorithm instance.
            return create_function(param);
        }
    }

private:
    /* Returns the factory instance */
    static Factory<T, Param> * instance()
    {
        static Factory<T> instance;
        return &instance;
    }

    std::map<Key, std::pair<CreateFunc ,CreateParam> > objects;
};

#endif
