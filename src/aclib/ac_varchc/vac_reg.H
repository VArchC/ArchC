/*  VArchC register proxy
    Copyright (C) 2018  Isaías Felzmann
    Visit http://varchc.github.io/ for more information.

    This module is part of ArchC
    Copyright (C) 2002-2018  The ArchC Team

    Please see license file distributed with this file.
    
    http://www.archc.org/
*/

#ifndef _VAC_REG_H_
#define _VAC_REG_H_

#include "ac_reg.H"
#include "vac_storage.H"
#include "vac_control.H"
#include <string>

namespace VArchC {
  namespace Storage {

    template <typename T>
    class Reg : public Word<T>{
      private:
        ac_reg<T> proxy;
        Control_base &vac_control;
        T faulty;
      public:
        T bypass;
        Reg(std::string _name, T _value, Control_base& control) : Word<T>(), proxy(_name, _value), vac_control(control), faulty(_value) {
          this->set(&(proxy.Data), &bypass, source_t::REG, &(proxy.Name), 0, &(vac_control.DM_regrd), &(vac_control.DM_regwr), vac_control.cur_instr_id);
        };

        const T& read() {
          uint64_t d = static_cast<uint64_t>(proxy.read());
          Encoder e(source_t::REG, proxy.Name, 0, source_t::READ, sizeof(T), d);
          vac_control.DM_regrd[*(vac_control.cur_instr_id)](&e);
          faulty = static_cast<T>(d);
          return faulty;
        }

        void write(T _data) {
          bypass = _data;
          uint64_t d = static_cast<uint64_t>(_data);
          Encoder e(source_t::REG, proxy.Name, 0, source_t::WRITE, sizeof(T), d);
          vac_control.DM_regwr[*(vac_control.cur_instr_id)](&e);
          proxy.write(static_cast<T>(d));
        }

        //operator const T&() {
        //  return read();
        //}

        //operator Word<T>&() {
        //  return data;
        //}


      ////////////////////////////
      // OPERATORS !!!          //
      ////////////////////////////



      //!Overloaded assign operators

      //Reg<T> = Reg
      Reg<T>& operator = ( const Reg<T> &r ){ 
        write( r.read() );
        return *this;
      }

      //Reg<T> = Type_T
      Reg<T> & operator = ( const T& d ){ 

        write( d );
        return *this;  
      }

      //!Reg<T> *= Type T
      Reg<T> & operator *= ( const T v ){ 
        write( read() * (int)v );
        return *this;
      }

      //!Reg<T> *= Reg
      Reg<T> & operator *= ( Reg<T> &r ){ 
        write( read() * r.read() );
        return *this;
      }

      //!Reg<T> /= Type T
      Reg<T> & operator /= ( const T d ){ 
        write( read() / d );
        return *this;
      }

      //!Reg<T> /= Reg
      Reg<T> & operator /= ( Reg<T> &r ){ 
        write( read() / r.read() );
        return *this;
      }

      //!Reg<T> %= Type T
      Reg<T> & operator %= ( const T d ){ 
        write( read() % d );
        return *this;
      }

      //!Reg<T> %= Reg
      Reg<T> & operator %= ( Reg<T> &r ){ 
        write( read() % r.read() );
        return *this;
      }

      //!Reg<T> += Type T
      Reg<T> & operator += ( const T& d ){ 
        write( read() + d );
        return *this;
      }

      //!Reg<T> += Reg
      Reg<T> & operator += ( Reg<T> &r ){ 
        write( read() + r.read() );
        return *this;
      }

      //!Reg<T> -= Type T
      Reg<T> & operator -= ( const T d ){ 
        write( read() - d );
        return *this;
      }

      //!Reg<T> -= Reg
      Reg<T> & operator -= ( Reg<T> &r ){ 
        write( read() - r.read() );
        return *this;
      }

      //!Reg<T> <<= Type T
      Reg<T> & operator <<= ( const T d ){ 
        write( read() << d );
        return *this;
      }

      //!Reg<T> <<= Reg
      Reg<T> & operator <<= ( Reg<T> &r ){ 
        write( read() << r.read() );
        return *this;
      }

      //!Reg<T> >>= Type T
      Reg<T> & operator >>= ( const T d ){ 
        write( read() >> d );
        return *this;
      }

      //!Reg<T> >>= Reg
      Reg<T> & operator >>= ( Reg<T> &r ){ 
        write( read() >> r.read() );
        return *this;
      }

      //!Reg<T> &= Type T
      Reg<T> & operator &= ( const T d ){ 
        write( read() & d );
        return *this;
      }

      //!Reg<T> &= Reg
      Reg<T> & operator &= ( Reg<T> &r ){ 
        write( read() & r.read() );
        return *this;
      }

      //!Reg<T> ^= Type T
      Reg<T> & operator ^= ( const T d ){ 
        write( read() ^ d );
        return *this;
      }

      //!Reg<T> ^= Reg
      Reg<T> & operator ^= ( Reg<T> &r ){ 
        write( read() ^ r.read() );
        return *this;
      }

      //!Reg<T> |= Type T
      Reg<T> & operator |= ( const T d ){ 
        write( read() | d );
        return *this;
      }

      //!Reg<T> |= Reg
      Reg<T> & operator |= ( Reg<T> &r ){ 
        write( read() | r.read() );
        return *this;
      }

#ifdef AC_DELAY
        Reg(std::string _name, T value, double&ts, Control_base& control) : proxy(name, value, ts), vac_control(control) {};

        void write(T _data, unsigned time) {
          bypass = _data;
          uint64_t d = static_cast<uint64_t>(_data);
          Encoder e(source_t::REG, proxy.Name, 0, source_t::WRITE, sizeof(T), d);
          vac_control.DM_regwr[*(vac_control.cur_instr_id)](&e);
          proxy.write(static_cast<T>(d), time);
        }

        void commit_delays(double time) {
          proxy.commit_delays(time);
        }

        Reg& operator =(ac_reg::chg_log _data) {
          proxy = _data;
          //data = proxy.read();
          return *this;
        }
#endif
      
    };

  }
}

#endif
