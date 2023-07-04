/* ************************************************************************
 * Copyright (C) 2018-2023 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#define ROCBLAS_BETA_FEATURES_API

#include "rocblas_data.hpp"
#include "rocblas_datatype2string.hpp"
#include "rocblas_test.hpp"
#include "testing_gemm_ex3.hpp"
#include "type_dispatch.hpp"
#include <cctype>
#include <cstring>
#include <type_traits>

namespace
{
    // Types of GEMM tests
    enum gemm_test_type
    {
        GEMM_EX3,
    };

    // ----------------------------------------------------------------------------
    // GEMM testing template
    // ----------------------------------------------------------------------------
    // The first template parameter is a class template which determines which
    // combination of types applies to this test, and for those that do, instantiates
    // the test code based on the function named in the test Arguments. The second
    // template parameter is an enum which allows the different flavors of GEMM to
    // be differentiated.
    //
    // The RocBLAS_Test base class takes this class (CRTP) and the first template
    // parameter as arguments, and provides common types such as type_filter_functor,
    // and derives from the Google Test parameterized base classes.
    //
    // This class defines functions for filtering the types and function names which
    // apply to this test, and for generating the suffix of the Google Test name
    // corresponding to each instance of this test.
    template <template <typename...> class FILTER, gemm_test_type GEMM_TYPE>
    struct gemm_test_template : RocBLAS_Test<gemm_test_template<FILTER, GEMM_TYPE>, FILTER>
    {
        // Filter for which types apply to this suite
        static bool type_filter(const Arguments& arg)
        {
            return rocblas_gemm_dispatch<gemm_test_template::template type_filter_functor>(arg);
        }

        // Filter for which functions apply to this suite
        static bool function_filter(const Arguments& arg)
        {
            switch(GEMM_TYPE)
            {

            case GEMM_EX3:
                return !strcmp(arg.function, "gemm_ex3")
                       || !strcmp(arg.function, "gemm_ex3_bad_arg");
            }

            return false;
        }

        // Google Test name suffix based on parameters
        static std::string name_suffix(const Arguments& arg)
        {
            RocBLAS_TestName<gemm_test_template> name(arg.name);
            name << rocblas_datatype2string(arg.a_type);

            name << rocblas_datatype2string(arg.b_type) << rocblas_datatype2string(arg.c_type)
                 << rocblas_datatype2string(arg.d_type)
                 << rocblas_computetype2string(arg.composite_compute_type);

            name << '_' << (char)std::toupper(arg.transA) << (char)std::toupper(arg.transB);

            name << '_' << arg.M << '_' << arg.N << '_' << arg.K << '_' << arg.alpha << '_'
                 << arg.lda << '_' << arg.ldb << '_' << arg.beta << '_' << arg.ldc;

            name << '_' << arg.ldd;

            return std::move(name);
        }
    };

    // ----------------------------------------------------------------------------
    // gemm_ex3
    // ----------------------------------------------------------------------------

    // In the general case of <Ti, To, Tc>, these tests do not apply, and if this
    // functor is called, an internal error message is generated. When converted
    // to bool, this functor returns false.
    template <typename TiA,
              typename TiB = TiA,
              typename To  = TiA,
              typename Tc  = To,
              typename     = void>
    struct gemm_ex3_testing : rocblas_test_invalid
    {
    };

    // When Ti != void, this test applies.
    // When converted to bool, this functor returns true.
    template <typename TiA, typename TiB, typename To, typename Tc>
    struct gemm_ex3_testing<
        TiA,
        TiB,
        To,
        Tc,
        std::enable_if_t<(!std::is_same<TiA, void>{} && !std::is_same<TiB, void>{})
                         && ((std::is_same<TiA, rocblas_f8>{} || std::is_same<TiA, rocblas_bf8>{}
                              || std::is_same<TiA, rocblas_half>{} || std::is_same<TiA, float>{}
                              || std::is_same<TiA, rocblas_bfloat16>{}))
                         && (std::is_same<TiB, rocblas_f8>{} || std::is_same<TiB, rocblas_bf8>{}
                             || std::is_same<TiB, rocblas_half>{} || std::is_same<TiB, float>{}
                             || std::is_same<TiB, rocblas_bfloat16>{})>> : rocblas_test_valid
    {
        void operator()(const Arguments& arg)
        {
            if(!strcmp(arg.function, "gemm_ex3"))
            {
                testing_gemm_ex3<TiA, TiB, To, Tc>(arg);
            }
            else if(!strcmp(arg.function, "gemm_ex3_bad_arg"))
            {
                testing_gemm_ex3_bad_arg<TiA, TiB, To, Tc>(arg);
            }
            else
                FAIL() << "Internal error: Test called with unknown function: " << arg.function;
        }
    };

    using gemm_ex3 = gemm_test_template<gemm_ex3_testing, GEMM_EX3>;
    TEST_P(gemm_ex3, blas3_tensile)
    {
        RUN_TEST_ON_THREADS_STREAMS(rocblas_gemm_dispatch<gemm_ex3_testing>(GetParam()));
    }
    INSTANTIATE_TEST_CATEGORIES(gemm_ex3);

} // namespace