/*
 * Copyright (c) 2011, 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#ifndef SHARE_VM_JVMCI_JVMCI_COMPILER_TO_VM_HPP
#define SHARE_VM_JVMCI_JVMCI_COMPILER_TO_VM_HPP

#include "gc/shared/cardTable.hpp"
#include "jvmci/jvmciExceptions.hpp"
#include "runtime/javaCalls.hpp"
#include "runtime/signature.hpp"

class JVMCIObjectArray;

class CompilerToVM {
 public:
  class Data {
    friend class JVMCIVMStructs;

   private:
    static int Klass_vtable_start_offset;
    static int Klass_vtable_length_offset;

    static int Method_extra_stack_entries;

    static address SharedRuntime_ic_miss_stub;
    static address SharedRuntime_handle_wrong_method_stub;
    static address SharedRuntime_deopt_blob_unpack;
    static address SharedRuntime_deopt_blob_unpack_with_exception_in_tls;
    static address SharedRuntime_deopt_blob_uncommon_trap;

    static size_t ThreadLocalAllocBuffer_alignment_reserve;

    static CollectedHeap* Universe_collectedHeap;
    static int Universe_base_vtable_size;
    static address Universe_narrow_oop_base;
    static int Universe_narrow_oop_shift;
    static address Universe_narrow_klass_base;
    static int Universe_narrow_klass_shift;
    static uintptr_t Universe_verify_oop_mask;
    static uintptr_t Universe_verify_oop_bits;
    static void* Universe_non_oop_bits;

    static bool _supports_inline_contig_alloc;
    static HeapWord** _heap_end_addr;
    static HeapWord* volatile* _heap_top_addr;
    static int _max_oop_map_stack_offset;
    static int _fields_annotations_base_offset;

    static jbyte* cardtable_start_address;
    static int cardtable_shift;

    static int vm_page_size;

    static int sizeof_vtableEntry;
    static int sizeof_ExceptionTableElement;
    static int sizeof_LocalVariableTableElement;
    static int sizeof_ConstantPool;
    static int sizeof_narrowKlass;
    static int sizeof_arrayOopDesc;
    static int sizeof_BasicLock;

    static address dsin;
    static address dcos;
    static address dtan;
    static address dexp;
    static address dlog;
    static address dlog10;
    static address dpow;

    static address symbol_init;
    static address symbol_clinit;

   public:
     static void initialize(JVMCI_TRAPS);

    static int max_oop_map_stack_offset() {
      assert(_max_oop_map_stack_offset > 0, "must be initialized");
      return Data::_max_oop_map_stack_offset;
    }
  };

  static bool cstring_equals(const char* const& s0, const char* const& s1) {
    return strcmp(s0, s1) == 0;
  }

  static unsigned cstring_hash(const char* const& s) {
    int h = 0;
    const char* p = s;
    while (*p != '\0') {
      h = 31 * h + *p;
      p++;
    }
    return h;
  }

  static JNINativeMethod methods[];
  static JNINativeMethod jni_methods[];

  static JVMCIObjectArray initialize_intrinsics(JVMCI_TRAPS);
 public:
  static int methods_count();

};


class JavaArgumentUnboxer : public SignatureIterator {
 protected:
  JavaCallArguments*  _jca;
  arrayOop _args;
  int _index;

  Handle next_arg(BasicType expectedType);

 public:
  JavaArgumentUnboxer(Symbol* signature, JavaCallArguments*  jca, arrayOop args, bool is_static) : SignatureIterator(signature) {
    this->_return_type = T_ILLEGAL;
    _jca = jca;
    _index = 0;
    _args = args;
    if (!is_static) {
      _jca->push_oop(next_arg(T_OBJECT));
    }
    iterate();
    assert(_index == args->length(), "arg count mismatch with signature");
  }

  inline void do_bool()   { if (!is_return_type()) _jca->push_int(next_arg(T_BOOLEAN)->bool_field(java_lang_boxing_object::value_offset_in_bytes(T_BOOLEAN))); }
  inline void do_char()   { if (!is_return_type()) _jca->push_int(next_arg(T_CHAR)->char_field(java_lang_boxing_object::value_offset_in_bytes(T_CHAR))); }
  inline void do_short()  { if (!is_return_type()) _jca->push_int(next_arg(T_SHORT)->short_field(java_lang_boxing_object::value_offset_in_bytes(T_SHORT))); }
  inline void do_byte()   { if (!is_return_type()) _jca->push_int(next_arg(T_BYTE)->byte_field(java_lang_boxing_object::value_offset_in_bytes(T_BYTE))); }
  inline void do_int()    { if (!is_return_type()) _jca->push_int(next_arg(T_INT)->int_field(java_lang_boxing_object::value_offset_in_bytes(T_INT))); }

  inline void do_long()   { if (!is_return_type()) _jca->push_long(next_arg(T_LONG)->long_field(java_lang_boxing_object::value_offset_in_bytes(T_LONG))); }
  inline void do_float()  { if (!is_return_type()) _jca->push_float(next_arg(T_FLOAT)->float_field(java_lang_boxing_object::value_offset_in_bytes(T_FLOAT))); }
  inline void do_double() { if (!is_return_type()) _jca->push_double(next_arg(T_DOUBLE)->double_field(java_lang_boxing_object::value_offset_in_bytes(T_DOUBLE))); }

  inline void do_object() { _jca->push_oop(next_arg(T_OBJECT)); }
  inline void do_object(int begin, int end) { if (!is_return_type()) _jca->push_oop(next_arg(T_OBJECT)); }
  inline void do_array(int begin, int end)  { if (!is_return_type()) _jca->push_oop(next_arg(T_OBJECT)); }
  inline void do_void()                     { }
};

class JNIHandleMark : public StackObj {
  JavaThread* _thread;
  public:
    JNIHandleMark(JavaThread* thread) : _thread(thread) { push_jni_handle_block(thread); }
    ~JNIHandleMark() { pop_jni_handle_block(_thread); }

  private:
    static void push_jni_handle_block(JavaThread* thread);
    static void pop_jni_handle_block(JavaThread* thread);
};

#endif // SHARE_VM_JVMCI_JVMCI_COMPILER_TO_VM_HPP
