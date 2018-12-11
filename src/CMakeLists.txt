cmake_minimum_required (VERSION 3.0)

function (AddExperimentMDP name)
    set(exename ${name}_main)
    add_executable(${exename} ${exename}.cpp)
    target_link_libraries(${exename} AIToolboxMDP AIToolboxFMDP ${Boost_LIBRARIES} ${ARGN})
endfunction (AddExperimentMDP)

function (AddExperimentPOMDP name)
    set(exename ${name}_main)
    add_executable(${exename} ${exename}.cpp)
    target_link_libraries(${exename} AIToolboxMDP AIToolboxPOMDP ${Boost_LIBRARIES} ${ARGN})
endfunction (AddExperimentPOMDP)

function (AddExperimentFMDP name)
    set(exename ${name}_main)
    add_executable(${exename} ${exename}.cpp)
    target_link_libraries(${exename} AIToolboxMDP AIToolboxFMDP ${Boost_LIBRARIES} ${ARGN})
endfunction (AddExperimentFMDP)

# Experiments should be in the format:
#
# experimentname_methodname

AddExperimentFMDP(mines_mauce)
AddExperimentFMDP(mines_scql)
AddExperimentFMDP(mines_llr)
AddExperimentMDP(mines_rnd)

AddExperimentFMDP(nodes_mauce)
AddExperimentFMDP(nodes_scql)
AddExperimentFMDP(nodes_llr)
AddExperimentMDP(nodes_rnd)

find_package(PythonLibs 2.7 REQUIRED)
include_directories(SYSTEM ${PYTHON_INCLUDE_DIRS})

AddExperimentFMDP(wind_mauce ${PYTHON_LIBRARIES})
AddExperimentFMDP(wind_scql ${PYTHON_LIBRARIES})
AddExperimentFMDP(wind_llr ${PYTHON_LIBRARIES})
AddExperimentFMDP(wind_rnd ${PYTHON_LIBRARIES})
