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

#include "agg_vcgen_markers_term.h"

namespace agg
{

    //------------------------------------------------------------------------
    void vcgen_markers_term::remove_all()
    {
        m_markers.remove_all();
    }


    //------------------------------------------------------------------------
    void vcgen_markers_term::add_vertex(float_t x, float_t y, unsigned cmd)
    {
        if(is_move_to(cmd))
        {
            if(m_markers.size() & 1)
            {
                // Initial state, the first coordinate was added.
                // If two of more calls of start_vertex() occures
                // we just modify the last one.
                m_markers.modify_last(coord_type(x, y));
            }
            else
            {
                m_markers.add(coord_type(x, y));
            }
        }
        else
        {
            if(is_vertex(cmd))
            {
                if(m_markers.size() & 1)
                {
                    // Initial state, the first coordinate was added.
                    // Add three more points, 0,1,1,0
                    m_markers.add(coord_type(x, y));
                    m_markers.add(m_markers[m_markers.size() - 1]);
                    m_markers.add(m_markers[m_markers.size() - 3]);
                }
                else
                {
                    if(m_markers.size())
                    {
                        // Replace two last points: 0,1,1,0 -> 0,1,2,1
                        m_markers[m_markers.size() - 1] = m_markers[m_markers.size() - 2];
                        m_markers[m_markers.size() - 2] = coord_type(x, y);
                    }
                }
            }
        }
    }


    //------------------------------------------------------------------------
    void vcgen_markers_term::rewind(unsigned path_id)
    {
        m_curr_id = path_id * 2;
        m_curr_idx = m_curr_id;
    }


    //------------------------------------------------------------------------
    unsigned vcgen_markers_term::vertex(float_t* x, float_t* y)
    {
        if(m_curr_id > 2 || m_curr_idx >= m_markers.size()) 
        {
            return path_cmd_stop;
        }
        const coord_type& c = m_markers[m_curr_idx];
        *x = c.x;
        *y = c.y;
        if(m_curr_idx & 1)
        {
            m_curr_idx += 3;
            return path_cmd_line_to;
        }
        ++m_curr_idx;
        return path_cmd_move_to;
    }


}
