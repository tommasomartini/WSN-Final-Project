#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-MacOSX
CND_DLIB_EXT=dylib
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/1360937237/blacklist_message.o \
	${OBJECTDIR}/_ext/1360937237/event.o \
	${OBJECTDIR}/_ext/1360937237/measure.o \
	${OBJECTDIR}/_ext/1360937237/message.o \
	${OBJECTDIR}/_ext/1360937237/my_toolbox.o \
	${OBJECTDIR}/_ext/1360937237/node.o \
	${OBJECTDIR}/_ext/1360937237/node_dispatcher.o \
	${OBJECTDIR}/_ext/1360937237/project_2.o \
	${OBJECTDIR}/_ext/1360937237/sensor_node.o \
	${OBJECTDIR}/_ext/1360937237/storage_node.o \
	${OBJECTDIR}/_ext/1360937237/storage_node_message.o \
	${OBJECTDIR}/_ext/1360937237/user.o \
	${OBJECTDIR}/_ext/1360937237/user_message.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/wsn

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/wsn: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/wsn ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/1360937237/blacklist_message.o: ../src/blacklist_message.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/blacklist_message.o ../src/blacklist_message.cpp

${OBJECTDIR}/_ext/1360937237/event.o: ../src/event.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/event.o ../src/event.cpp

${OBJECTDIR}/_ext/1360937237/measure.o: ../src/measure.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/measure.o ../src/measure.cpp

${OBJECTDIR}/_ext/1360937237/message.o: ../src/message.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/message.o ../src/message.cpp

${OBJECTDIR}/_ext/1360937237/my_toolbox.o: ../src/my_toolbox.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/my_toolbox.o ../src/my_toolbox.cpp

${OBJECTDIR}/_ext/1360937237/node.o: ../src/node.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/node.o ../src/node.cpp

${OBJECTDIR}/_ext/1360937237/node_dispatcher.o: ../src/node_dispatcher.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/node_dispatcher.o ../src/node_dispatcher.cpp

${OBJECTDIR}/_ext/1360937237/project_2.o: ../src/project_2.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/project_2.o ../src/project_2.cpp

${OBJECTDIR}/_ext/1360937237/sensor_node.o: ../src/sensor_node.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/sensor_node.o ../src/sensor_node.cpp

${OBJECTDIR}/_ext/1360937237/storage_node.o: ../src/storage_node.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/storage_node.o ../src/storage_node.cpp

${OBJECTDIR}/_ext/1360937237/storage_node_message.o: ../src/storage_node_message.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/storage_node_message.o ../src/storage_node_message.cpp

${OBJECTDIR}/_ext/1360937237/user.o: ../src/user.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/user.o ../src/user.cpp

${OBJECTDIR}/_ext/1360937237/user_message.o: ../src/user_message.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1360937237/user_message.o ../src/user_message.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/wsn

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
