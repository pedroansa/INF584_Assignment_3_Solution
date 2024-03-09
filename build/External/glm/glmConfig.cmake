set(GLM_VERSION "0.9.9")
set(GLM_INCLUDE_DIRS "/home/tintin/Projets/C++/SFML2.5/OGL4/INF584_Assignment_3_Solution/External/glm")

if (NOT CMAKE_VERSION VERSION_LESS "3.0")
    include("${CMAKE_CURRENT_LIST_DIR}/glmTargets.cmake")
endif()
