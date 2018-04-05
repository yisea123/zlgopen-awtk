//----------------------------------------------------------------------------
// Anti-Grain Geometry (AGG) - Version 2.5
// A high quality rendering engine for C++
// Copyright (C) 2002-2006 Maxim Shemanarev
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://antigrain.com
// 
// AGG is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// AGG is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AGG; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
// MA 02110-1301, USA.
//----------------------------------------------------------------------------

#ifndef AGG_VERTEX_SEQUENCE_INCLUDED
#define AGG_VERTEX_SEQUENCE_INCLUDED

#include "agg_basics.h"
#include "agg_array.h"
#include "agg_math.h"

namespace agg
{

    //----------------------------------------------------------vertex_sequence
    // Modified agg::pod_bvector. The data is interpreted as a sequence 
    // of vertices. It means that the type T must expose:
    //
    // bool T::operator() (const T& val)
    // 
    // that is called every time new vertex is being added. The main purpose
    // of this operator is the possibility to calculate some values during 
    // adding and to return true if the vertex fits some criteria or false if
    // it doesn't. In the last case the new vertex is not added. 
    // 
    // The simple example is filtering coinciding vertices with calculation 
    // of the distance between the current and previous ones:
    //
    //    struct vertex_dist
    //    {
    //        float_t   x;
    //        float_t   y;
    //        float_t   dist;
    //
    //        vertex_dist() {}
    //        vertex_dist(float_t x_, float_t y_) :
    //            x(x_),
    //            y(y_),
    //            dist(0.0)
    //        {
    //        }
    //
    //        bool operator () (const vertex_dist& val)
    //        {
    //            return (dist = calc_distance(x, y, val.x, val.y)) > EPSILON;
    //        }
    //    };
    //
    // Function close() calls this operator and removes the last vertex if 
    // necessary.
    //------------------------------------------------------------------------
    template<class T, unsigned S=6> 
    class vertex_sequence : public pod_bvector<T, S>
    {
    public:
        typedef pod_bvector<T, S> base_type;

        void add(const T& val);
        void modify_last(const T& val);
        void close(bool remove_flag);
    };



    //------------------------------------------------------------------------
    template<class T, unsigned S> 
    void vertex_sequence<T, S>::add(const T& val)
    {
        if(base_type::size() > 1)
        {
            if(!(*this)[base_type::size() - 2]((*this)[base_type::size() - 1])) 
            {
                base_type::remove_last();
            }
        }
        base_type::add(val);
    }


    //------------------------------------------------------------------------
    template<class T, unsigned S> 
    void vertex_sequence<T, S>::modify_last(const T& val)
    {
        base_type::remove_last();
        add(val);
    }



    //------------------------------------------------------------------------
    template<class T, unsigned S> 
    void vertex_sequence<T, S>::close(bool closed)
    {
        while(base_type::size() > 1)
        {
            if((*this)[base_type::size() - 2]((*this)[base_type::size() - 1])) break;
            T t = (*this)[base_type::size() - 1];
            base_type::remove_last();
            modify_last(t);
        }

        if(closed)
        {
            while(base_type::size() > 1)
            {
                if((*this)[base_type::size() - 1]((*this)[0])) break;
                base_type::remove_last();
            }
        }
    }


    //-------------------------------------------------------------vertex_dist
    // Vertex (x, y) with the distance to the next one. The last vertex has 
    // distance between the last and the first points if the polygon is closed
    // and 0.0 if it's a polyline.
    struct vertex_dist
    {
        float_t   x;
        float_t   y;
        float_t   dist;

        vertex_dist() {}
        vertex_dist(float_t x_, float_t y_) :
            x(x_),
            y(y_),
            dist(0.0)
        {
        }

        bool operator () (const vertex_dist& val)
        {
            bool ret = (dist = calc_distance(x, y, val.x, val.y)) > vertex_dist_epsilon;
            if(!ret) dist = 1.0 / vertex_dist_epsilon;
            return ret;
        }
    };



    //--------------------------------------------------------vertex_dist_cmd
    // Save as the above but with additional "command" value
    struct vertex_dist_cmd : public vertex_dist
    {
        unsigned cmd;

        vertex_dist_cmd() {}
        vertex_dist_cmd(float_t x_, float_t y_, unsigned cmd_) :
            vertex_dist(x_, y_),
            cmd(cmd_)
        {
        }
    };


}

#endif
