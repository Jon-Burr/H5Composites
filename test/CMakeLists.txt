find_package(Boost 1.66 COMPONENTS unit_test_framework REQUIRED)

function(define_utest name)
    add_executable(test_${name} ${name}.cxx)
    target_link_libraries(test_${name} PRIVATE H5Composites Boost::unit_test_framework)
    add_test(${name} test_${name})
endfunction()

define_utest(dtypes)
define_utest(iterator)
define_utest(struct)

# define_utest(readwrite_primitives)
# define_utest(array)
# define_utest(tuple)