/*
 * UTResponseMatrixOrderShared.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

#ifndef __UTResponseMatrixOrderShared__
#define __UTResponseMatrixOrderShared__

// Standard libs:
#include <cstdlib>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

// MEGAlib:
#include "MFile.h"
#include "MStreams.h"
#include "MUnitTest.h"

// ROOT:
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TROOT.h>


inline bool PrepareResponseMatrixTempDirectory()
{
  return system("mkdir -p /tmp/UTResponseMatrix") == 0;
}


inline bool RunAssertingUnitTest(const MString& Executable, const MString& Argument)
{
  pid_t Child = fork();
  if (Child == 0) {
    int Log = open("/tmp/UTResponseMatrix/response_matrix_assert.log", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (Log >= 0) {
      dup2(Log, STDOUT_FILENO);
      dup2(Log, STDERR_FILENO);
      close(Log);
    }
    execl(Executable.Data(), Executable.Data(), Argument.Data(), static_cast<char*>(0));
    _exit(127);
  }

  if (Child < 0) {
    return false;
  }

  int Status = 0;
  if (waitpid(Child, &Status, 0) < 0) {
    return false;
  }

  return WIFSIGNALED(Status) || (WIFEXITED(Status) && WEXITSTATUS(Status) != 0);
}


inline int GetCanvasCount()
{
  return gROOT != 0 && gROOT->GetListOfCanvases() != 0 ? gROOT->GetListOfCanvases()->GetSize() : 0;
}


inline void CleanupCanvases(int TargetCount)
{
  while (gROOT != 0 && gROOT->GetListOfCanvases() != 0 && gROOT->GetListOfCanvases()->GetSize() > TargetCount) {
    TCanvas* Canvas = dynamic_cast<TCanvas*>(gROOT->GetListOfCanvases()->Last());
    if (Canvas == 0) {
      break;
    }
    delete Canvas;
  }
}


#define RM_DECLARE_AXES_2 \
  vector<float> A1{0.0f, 1.0f, 2.0f}; \
  vector<float> A2{0.0f, 1.0f, 2.0f}; \
  vector<float> ADifferent{0.0f, 1.0f, 3.0f};
#define RM_DECLARE_AXES_3 RM_DECLARE_AXES_2 vector<float> A3{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_4 RM_DECLARE_AXES_3 vector<float> A4{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_5 RM_DECLARE_AXES_4 vector<float> A5{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_6 RM_DECLARE_AXES_5 vector<float> A6{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_7 RM_DECLARE_AXES_6 vector<float> A7{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_8 RM_DECLARE_AXES_7 vector<float> A8{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_9 RM_DECLARE_AXES_8 vector<float> A9{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_10 RM_DECLARE_AXES_9 vector<float> A10{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_11 RM_DECLARE_AXES_10 vector<float> A11{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_12 RM_DECLARE_AXES_11 vector<float> A12{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_13 RM_DECLARE_AXES_12 vector<float> A13{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_14 RM_DECLARE_AXES_13 vector<float> A14{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_15 RM_DECLARE_AXES_14 vector<float> A15{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_16 RM_DECLARE_AXES_15 vector<float> A16{0.0f, 1.0f, 2.0f};
#define RM_DECLARE_AXES_17 RM_DECLARE_AXES_16 vector<float> A17{0.0f, 1.0f, 2.0f};

#define RM_AXIS_ARGS_2 A1, A2
#define RM_AXIS_ARGS_3 A1, A2, A3
#define RM_AXIS_ARGS_4 A1, A2, A3, A4
#define RM_AXIS_ARGS_5 A1, A2, A3, A4, A5
#define RM_AXIS_ARGS_6 A1, A2, A3, A4, A5, A6
#define RM_AXIS_ARGS_7 A1, A2, A3, A4, A5, A6, A7
#define RM_AXIS_ARGS_8 A1, A2, A3, A4, A5, A6, A7, A8
#define RM_AXIS_ARGS_9 A1, A2, A3, A4, A5, A6, A7, A8, A9
#define RM_AXIS_ARGS_10 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10
#define RM_AXIS_ARGS_11 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11
#define RM_AXIS_ARGS_12 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12
#define RM_AXIS_ARGS_13 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13
#define RM_AXIS_ARGS_14 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14
#define RM_AXIS_ARGS_15 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15
#define RM_AXIS_ARGS_16 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16
#define RM_AXIS_ARGS_17 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17

#define RM_AXIS_ARGS_PREV_2 A1
#define RM_AXIS_ARGS_PREV_3 A1, A2
#define RM_AXIS_ARGS_PREV_4 A1, A2, A3
#define RM_AXIS_ARGS_PREV_5 A1, A2, A3, A4
#define RM_AXIS_ARGS_PREV_6 A1, A2, A3, A4, A5
#define RM_AXIS_ARGS_PREV_7 A1, A2, A3, A4, A5, A6
#define RM_AXIS_ARGS_PREV_8 A1, A2, A3, A4, A5, A6, A7
#define RM_AXIS_ARGS_PREV_9 A1, A2, A3, A4, A5, A6, A7, A8
#define RM_AXIS_ARGS_PREV_10 A1, A2, A3, A4, A5, A6, A7, A8, A9
#define RM_AXIS_ARGS_PREV_11 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10
#define RM_AXIS_ARGS_PREV_12 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11
#define RM_AXIS_ARGS_PREV_13 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12
#define RM_AXIS_ARGS_PREV_14 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13
#define RM_AXIS_ARGS_PREV_15 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14
#define RM_AXIS_ARGS_PREV_16 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15
#define RM_AXIS_ARGS_PREV_17 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16

#define RM_AXIS_ARGS_DIFFERENT_2 A1, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_3 A1, A2, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_4 A1, A2, A3, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_5 A1, A2, A3, A4, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_6 A1, A2, A3, A4, A5, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_7 A1, A2, A3, A4, A5, A6, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_8 A1, A2, A3, A4, A5, A6, A7, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_9 A1, A2, A3, A4, A5, A6, A7, A8, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_10 A1, A2, A3, A4, A5, A6, A7, A8, A9, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_11 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_12 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_13 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_14 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_15 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_16 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, ADifferent
#define RM_AXIS_ARGS_DIFFERENT_17 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, ADifferent

#define RM_AXIS_NAME_ARGS_2 MString("X1"), MString("X2")
#define RM_AXIS_NAME_ARGS_3 RM_AXIS_NAME_ARGS_2, MString("X3")
#define RM_AXIS_NAME_ARGS_4 RM_AXIS_NAME_ARGS_3, MString("X4")
#define RM_AXIS_NAME_ARGS_5 RM_AXIS_NAME_ARGS_4, MString("X5")
#define RM_AXIS_NAME_ARGS_6 RM_AXIS_NAME_ARGS_5, MString("X6")
#define RM_AXIS_NAME_ARGS_7 RM_AXIS_NAME_ARGS_6, MString("X7")
#define RM_AXIS_NAME_ARGS_8 RM_AXIS_NAME_ARGS_7, MString("X8")
#define RM_AXIS_NAME_ARGS_9 RM_AXIS_NAME_ARGS_8, MString("X9")
#define RM_AXIS_NAME_ARGS_10 RM_AXIS_NAME_ARGS_9, MString("X10")
#define RM_AXIS_NAME_ARGS_11 RM_AXIS_NAME_ARGS_10, MString("X11")
#define RM_AXIS_NAME_ARGS_12 RM_AXIS_NAME_ARGS_11, MString("X12")
#define RM_AXIS_NAME_ARGS_13 RM_AXIS_NAME_ARGS_12, MString("X13")
#define RM_AXIS_NAME_ARGS_14 RM_AXIS_NAME_ARGS_13, MString("X14")
#define RM_AXIS_NAME_ARGS_15 RM_AXIS_NAME_ARGS_14, MString("X15")
#define RM_AXIS_NAME_ARGS_16 RM_AXIS_NAME_ARGS_15, MString("X16")
#define RM_AXIS_NAME_ARGS_17 RM_AXIS_NAME_ARGS_16, MString("X17")

#define RM_AXIS_NAME_ARGS_PREV_2 MString("X1")
#define RM_AXIS_NAME_ARGS_PREV_3 RM_AXIS_NAME_ARGS_2
#define RM_AXIS_NAME_ARGS_PREV_4 RM_AXIS_NAME_ARGS_3
#define RM_AXIS_NAME_ARGS_PREV_5 RM_AXIS_NAME_ARGS_4
#define RM_AXIS_NAME_ARGS_PREV_6 RM_AXIS_NAME_ARGS_5
#define RM_AXIS_NAME_ARGS_PREV_7 RM_AXIS_NAME_ARGS_6
#define RM_AXIS_NAME_ARGS_PREV_8 RM_AXIS_NAME_ARGS_7
#define RM_AXIS_NAME_ARGS_PREV_9 RM_AXIS_NAME_ARGS_8
#define RM_AXIS_NAME_ARGS_PREV_10 RM_AXIS_NAME_ARGS_9
#define RM_AXIS_NAME_ARGS_PREV_11 RM_AXIS_NAME_ARGS_10
#define RM_AXIS_NAME_ARGS_PREV_12 RM_AXIS_NAME_ARGS_11
#define RM_AXIS_NAME_ARGS_PREV_13 RM_AXIS_NAME_ARGS_12
#define RM_AXIS_NAME_ARGS_PREV_14 RM_AXIS_NAME_ARGS_13
#define RM_AXIS_NAME_ARGS_PREV_15 RM_AXIS_NAME_ARGS_14
#define RM_AXIS_NAME_ARGS_PREV_16 RM_AXIS_NAME_ARGS_15
#define RM_AXIS_NAME_ARGS_PREV_17 RM_AXIS_NAME_ARGS_16

#define RM_AXIS_ARGS_EMPTY_FIRST_2 EmptyAxis, A2
#define RM_AXIS_ARGS_EMPTY_FIRST_3 EmptyAxis, A2, A3
#define RM_AXIS_ARGS_EMPTY_FIRST_4 EmptyAxis, A2, A3, A4
#define RM_AXIS_ARGS_EMPTY_FIRST_5 EmptyAxis, A2, A3, A4, A5
#define RM_AXIS_ARGS_EMPTY_FIRST_6 EmptyAxis, A2, A3, A4, A5, A6
#define RM_AXIS_ARGS_EMPTY_FIRST_7 EmptyAxis, A2, A3, A4, A5, A6, A7
#define RM_AXIS_ARGS_EMPTY_FIRST_8 EmptyAxis, A2, A3, A4, A5, A6, A7, A8
#define RM_AXIS_ARGS_EMPTY_FIRST_9 EmptyAxis, A2, A3, A4, A5, A6, A7, A8, A9
#define RM_AXIS_ARGS_EMPTY_FIRST_10 EmptyAxis, A2, A3, A4, A5, A6, A7, A8, A9, A10
#define RM_AXIS_ARGS_EMPTY_FIRST_11 EmptyAxis, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11
#define RM_AXIS_ARGS_EMPTY_FIRST_12 EmptyAxis, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12
#define RM_AXIS_ARGS_EMPTY_FIRST_13 EmptyAxis, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13
#define RM_AXIS_ARGS_EMPTY_FIRST_14 EmptyAxis, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14
#define RM_AXIS_ARGS_EMPTY_FIRST_15 EmptyAxis, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15
#define RM_AXIS_ARGS_EMPTY_FIRST_16 EmptyAxis, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16
#define RM_AXIS_ARGS_EMPTY_FIRST_17 EmptyAxis, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17

#define RM_AXIS_ARGS_NONINCREASING_LAST_2 A1, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_3 A1, A2, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_4 A1, A2, A3, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_5 A1, A2, A3, A4, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_6 A1, A2, A3, A4, A5, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_7 A1, A2, A3, A4, A5, A6, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_8 A1, A2, A3, A4, A5, A6, A7, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_9 A1, A2, A3, A4, A5, A6, A7, A8, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_10 A1, A2, A3, A4, A5, A6, A7, A8, A9, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_11 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_12 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_13 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_14 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_15 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_16 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, NonIncreasingAxis
#define RM_AXIS_ARGS_NONINCREASING_LAST_17 A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, NonIncreasingAxis

#define RM_ZERO_BINS_2 0, 0
#define RM_ZERO_BINS_3 RM_ZERO_BINS_2, 0
#define RM_ZERO_BINS_4 RM_ZERO_BINS_3, 0
#define RM_ZERO_BINS_5 RM_ZERO_BINS_4, 0
#define RM_ZERO_BINS_6 RM_ZERO_BINS_5, 0
#define RM_ZERO_BINS_7 RM_ZERO_BINS_6, 0
#define RM_ZERO_BINS_8 RM_ZERO_BINS_7, 0
#define RM_ZERO_BINS_9 RM_ZERO_BINS_8, 0
#define RM_ZERO_BINS_10 RM_ZERO_BINS_9, 0
#define RM_ZERO_BINS_11 RM_ZERO_BINS_10, 0
#define RM_ZERO_BINS_12 RM_ZERO_BINS_11, 0
#define RM_ZERO_BINS_13 RM_ZERO_BINS_12, 0
#define RM_ZERO_BINS_14 RM_ZERO_BINS_13, 0
#define RM_ZERO_BINS_15 RM_ZERO_BINS_14, 0
#define RM_ZERO_BINS_16 RM_ZERO_BINS_15, 0
#define RM_ZERO_BINS_17 RM_ZERO_BINS_16, 0

#define RM_ZERO_BINS_PREV_2 0
#define RM_ZERO_BINS_PREV_3 RM_ZERO_BINS_2
#define RM_ZERO_BINS_PREV_4 RM_ZERO_BINS_3
#define RM_ZERO_BINS_PREV_5 RM_ZERO_BINS_4
#define RM_ZERO_BINS_PREV_6 RM_ZERO_BINS_5
#define RM_ZERO_BINS_PREV_7 RM_ZERO_BINS_6
#define RM_ZERO_BINS_PREV_8 RM_ZERO_BINS_7
#define RM_ZERO_BINS_PREV_9 RM_ZERO_BINS_8
#define RM_ZERO_BINS_PREV_10 RM_ZERO_BINS_9
#define RM_ZERO_BINS_PREV_11 RM_ZERO_BINS_10
#define RM_ZERO_BINS_PREV_12 RM_ZERO_BINS_11
#define RM_ZERO_BINS_PREV_13 RM_ZERO_BINS_12
#define RM_ZERO_BINS_PREV_14 RM_ZERO_BINS_13
#define RM_ZERO_BINS_PREV_15 RM_ZERO_BINS_14
#define RM_ZERO_BINS_PREV_16 RM_ZERO_BINS_15
#define RM_ZERO_BINS_PREV_17 RM_ZERO_BINS_16

#define RM_VALUE_COORDS_2 0.25f, 0.25f
#define RM_VALUE_COORDS_3 RM_VALUE_COORDS_2, 0.25f
#define RM_VALUE_COORDS_4 RM_VALUE_COORDS_3, 0.25f
#define RM_VALUE_COORDS_5 RM_VALUE_COORDS_4, 0.25f
#define RM_VALUE_COORDS_6 RM_VALUE_COORDS_5, 0.25f
#define RM_VALUE_COORDS_7 RM_VALUE_COORDS_6, 0.25f
#define RM_VALUE_COORDS_8 RM_VALUE_COORDS_7, 0.25f
#define RM_VALUE_COORDS_9 RM_VALUE_COORDS_8, 0.25f
#define RM_VALUE_COORDS_10 RM_VALUE_COORDS_9, 0.25f
#define RM_VALUE_COORDS_11 RM_VALUE_COORDS_10, 0.25f
#define RM_VALUE_COORDS_12 RM_VALUE_COORDS_11, 0.25f
#define RM_VALUE_COORDS_13 RM_VALUE_COORDS_12, 0.25f
#define RM_VALUE_COORDS_14 RM_VALUE_COORDS_13, 0.25f
#define RM_VALUE_COORDS_15 RM_VALUE_COORDS_14, 0.25f
#define RM_VALUE_COORDS_16 RM_VALUE_COORDS_15, 0.25f
#define RM_VALUE_COORDS_17 RM_VALUE_COORDS_16, 0.25f

#define RM_CENTER_COORDS_2 0.5f, 0.5f
#define RM_CENTER_COORDS_3 RM_CENTER_COORDS_2, 0.5f
#define RM_CENTER_COORDS_4 RM_CENTER_COORDS_3, 0.5f
#define RM_CENTER_COORDS_5 RM_CENTER_COORDS_4, 0.5f
#define RM_CENTER_COORDS_6 RM_CENTER_COORDS_5, 0.5f
#define RM_CENTER_COORDS_7 RM_CENTER_COORDS_6, 0.5f
#define RM_CENTER_COORDS_8 RM_CENTER_COORDS_7, 0.5f
#define RM_CENTER_COORDS_9 RM_CENTER_COORDS_8, 0.5f
#define RM_CENTER_COORDS_10 RM_CENTER_COORDS_9, 0.5f
#define RM_CENTER_COORDS_11 RM_CENTER_COORDS_10, 0.5f
#define RM_CENTER_COORDS_12 RM_CENTER_COORDS_11, 0.5f
#define RM_CENTER_COORDS_13 RM_CENTER_COORDS_12, 0.5f
#define RM_CENTER_COORDS_14 RM_CENTER_COORDS_13, 0.5f
#define RM_CENTER_COORDS_15 RM_CENTER_COORDS_14, 0.5f
#define RM_CENTER_COORDS_16 RM_CENTER_COORDS_15, 0.5f
#define RM_CENTER_COORDS_17 RM_CENTER_COORDS_16, 0.5f

#define RM_HIST_ARGS_2 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY
#define RM_HIST_ARGS_3 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f
#define RM_HIST_ARGS_4 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f
#define RM_HIST_ARGS_5 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_6 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_7 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_8 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_9 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_10 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_11 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_12 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_13 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_14 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_15 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_16 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f
#define RM_HIST_ARGS_17 MResponseMatrix::c_ShowX, MResponseMatrix::c_ShowY, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f

#define RM_HISTOGRAM_TESTS_3 \
    DisableDefaultStreams(); \
    TH1* Histogram = Matrix.GetHistogram(RM_HIST_ARGS_3, false); \
    EnableDefaultStreams(); \
    Passed = EvaluateTrue("GetHistogram()", "representative histogram", "GetHistogram returns a representative ROOT histogram", Histogram != nullptr) && Passed; \
    if (Histogram != nullptr) { \
      Passed = EvaluateTrue("GetHistogram()", "representative histogram dimensionality", "GetHistogram returns a two-dimensional histogram when the first two axes are selected for display", dynamic_cast<TH2*>(Histogram) != nullptr) && Passed; \
      Passed = EvaluateNear("GetHistogram()", "representative histogram content", "GetHistogram preserves the representative first-bin content", Histogram->GetBinContent(1, 1), 6.0, 1e-6) && Passed; \
      delete Histogram; \
    }

#define RM_HISTOGRAM_TESTS_4 \
    DisableDefaultStreams(); \
    TH1* Histogram = Matrix.GetHistogram(RM_HIST_ARGS_4, false); \
    EnableDefaultStreams(); \
    Passed = EvaluateTrue("GetHistogram()", "representative histogram", "GetHistogram returns a representative ROOT histogram", Histogram != nullptr) && Passed; \
    if (Histogram != nullptr) { \
      Passed = EvaluateTrue("GetHistogram()", "representative histogram dimensionality", "GetHistogram returns a two-dimensional histogram when the first two axes are selected for display", dynamic_cast<TH2*>(Histogram) != nullptr) && Passed; \
      Passed = EvaluateNear("GetHistogram()", "representative histogram content", "GetHistogram preserves the representative first-bin content", Histogram->GetBinContent(1, 1), 6.0, 1e-6) && Passed; \
      delete Histogram; \
    }

#define RM_HISTOGRAM_TESTS_5 \
    DisableDefaultStreams(); \
    TH1* Histogram = Matrix.GetHistogram(RM_HIST_ARGS_5, false); \
    EnableDefaultStreams(); \
    Passed = EvaluateTrue("GetHistogram()", "representative histogram", "GetHistogram returns a representative ROOT histogram", Histogram != nullptr) && Passed; \
    if (Histogram != nullptr) { \
      Passed = EvaluateTrue("GetHistogram()", "representative histogram dimensionality", "GetHistogram returns a two-dimensional histogram when the first two axes are selected for display", dynamic_cast<TH2*>(Histogram) != nullptr) && Passed; \
      Passed = EvaluateNear("GetHistogram()", "representative histogram content", "GetHistogram preserves the representative first-bin content", Histogram->GetBinContent(1, 1), 6.0, 1e-6) && Passed; \
      delete Histogram; \
    }

#define RM_HISTOGRAM_TESTS_6
#define RM_HISTOGRAM_TESTS_7
#define RM_HISTOGRAM_TESTS_8
#define RM_HISTOGRAM_TESTS_9
#define RM_HISTOGRAM_TESTS_10
#define RM_HISTOGRAM_TESTS_11
#define RM_HISTOGRAM_TESTS_12
#define RM_HISTOGRAM_TESTS_13
#define RM_HISTOGRAM_TESTS_14
#define RM_HISTOGRAM_TESTS_15
#define RM_HISTOGRAM_TESTS_16
#define RM_HISTOGRAM_TESTS_17

#define RM_ORDER_INDICES_2 1, 2
#define RM_ORDER_INDICES_3 RM_ORDER_INDICES_2, 3
#define RM_ORDER_INDICES_4 RM_ORDER_INDICES_3, 4
#define RM_ORDER_INDICES_5 RM_ORDER_INDICES_4, 5
#define RM_ORDER_INDICES_6 RM_ORDER_INDICES_5, 6
#define RM_ORDER_INDICES_7 RM_ORDER_INDICES_6, 7
#define RM_ORDER_INDICES_8 RM_ORDER_INDICES_7, 8
#define RM_ORDER_INDICES_9 RM_ORDER_INDICES_8, 9
#define RM_ORDER_INDICES_10 RM_ORDER_INDICES_9, 10
#define RM_ORDER_INDICES_11 RM_ORDER_INDICES_10, 11
#define RM_ORDER_INDICES_12 RM_ORDER_INDICES_11, 12
#define RM_ORDER_INDICES_13 RM_ORDER_INDICES_12, 13
#define RM_ORDER_INDICES_14 RM_ORDER_INDICES_13, 14
#define RM_ORDER_INDICES_15 RM_ORDER_INDICES_14, 15
#define RM_ORDER_INDICES_16 RM_ORDER_INDICES_15, 16
#define RM_ORDER_INDICES_17 RM_ORDER_INDICES_16, 17

#define RM_EXTRA_TESTS_2 \
    MResponseMatrixO2 Sum2_2 = Matrix.GetSumMatrixO2(1, 2); \
    Passed = EvaluateNear("GetSumMatrixO2()", "identity projection", "The order-2 sum projection preserves the representative first-bin content", Sum2_2.GetBinContent(0, 0), 6.0, 1e-6) && Passed; \
    MResponseMatrixO1 Slice2 = Matrix.GetSliceInterpolated(0.5f, 2); \
    Passed = EvaluateNear("GetSliceInterpolated()", "representative slice", "The representative order-1 slice preserves the first-bin content", Slice2.GetBinContent(0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_3 \
    MResponseMatrixO2 Sum2_3 = Matrix.GetSumMatrixO2(1, 2); \
    Passed = EvaluateNear("GetSumMatrixO2()", "first two axes", "The order-2 sum projection preserves the representative first-bin content", Sum2_3.GetBinContent(0, 0), 6.0, 1e-6) && Passed; \
    MResponseMatrixO1 Slice3 = Matrix.GetSliceInterpolated(0.5f, 2, 0.5f, 3); \
    Passed = EvaluateNear("GetSliceInterpolated()", "representative slice", "The representative order-1 slice preserves the first-bin content", Slice3.GetBinContent(0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_4 \
    MResponseMatrixO2 Sum2_4 = Matrix.GetSumMatrixO2(1, 2); \
    Passed = EvaluateNear("GetSumMatrixO2()", "first two axes", "The order-2 sum projection preserves the representative first-bin content", Sum2_4.GetBinContent(0, 0), 6.0, 1e-6) && Passed; \
    MResponseMatrixO3 Sum3_4 = Matrix.GetSumMatrixO3(1, 2, 3); \
    Passed = EvaluateNear("GetSumMatrixO3()", "first three axes", "The order-3 sum projection preserves the representative first-bin content", Sum3_4.GetBinContent(0, 0, 0), 6.0, 1e-6) && Passed; \
    MResponseMatrixO1 Slice4 = Matrix.GetSliceInterpolated(0.5f, 2, 0.5f, 3, 0.5f, 4); \
    Passed = EvaluateNear("GetSliceInterpolated()", "representative slice", "The representative order-1 slice preserves the first-bin content", Slice4.GetBinContent(0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_5 \
    MResponseMatrixO2 Sum2_5 = Matrix.GetSumMatrixO2(1, 2); \
    Passed = EvaluateNear("GetSumMatrixO2()", "first two axes", "The order-2 sum projection preserves the representative first-bin content", Sum2_5.GetBinContent(0, 0), 6.0, 1e-6) && Passed; \
    MResponseMatrixO3 Sum3_5 = Matrix.GetSumMatrixO3(1, 2, 3); \
    Passed = EvaluateNear("GetSumMatrixO3()", "first three axes", "The order-3 sum projection preserves the representative first-bin content", Sum3_5.GetBinContent(0, 0, 0), 6.0, 1e-6) && Passed; \
    MResponseMatrixO4 Sum4_5 = Matrix.GetSumMatrixO4(1, 2, 3, 4); \
    Passed = EvaluateNear("GetSumMatrixO4()", "first four axes", "The order-4 sum projection preserves the representative first-bin content", Sum4_5.GetBinContent(0, 0, 0, 0), 6.0, 1e-6) && Passed; \
    MResponseMatrixO1 Slice5 = Matrix.GetSliceInterpolated(0.5f, 2, 0.5f, 3, 0.5f, 4, 0.5f, 5); \
    Passed = EvaluateNear("GetSliceInterpolated()", "representative slice", "The representative order-1 slice preserves the first-bin content", Slice5.GetBinContent(0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_6 \
    MResponseMatrixO2 Sum2_6 = Matrix.GetSumMatrixO2(1, 2); \
    Passed = EvaluateNear("GetSumMatrixO2()", "first two axes", "The order-2 sum projection preserves the representative first-bin content", Sum2_6.GetBinContent(0, 0), 6.0, 1e-6) && Passed; \
    MResponseMatrixO3 Sum3_6 = Matrix.GetSumMatrixO3(1, 2, 3); \
    Passed = EvaluateNear("GetSumMatrixO3()", "first three axes", "The order-3 sum projection preserves the representative first-bin content", Sum3_6.GetBinContent(0, 0, 0), 6.0, 1e-6) && Passed; \
    MResponseMatrixO4 Sum4_6 = Matrix.GetSumMatrixO4(1, 2, 3, 4); \
    Passed = EvaluateNear("GetSumMatrixO4()", "first four axes", "The order-4 sum projection preserves the representative first-bin content", Sum4_6.GetBinContent(0, 0, 0, 0), 6.0, 1e-6) && Passed; \
    MResponseMatrixO5 Sum5_6 = Matrix.GetSumMatrixO5(1, 2, 3, 4, 5); \
    Passed = EvaluateNear("GetSumMatrixO5()", "first five axes", "The order-5 sum projection preserves the representative first-bin content", Sum5_6.GetBinContent(0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_7 \
    RM_EXTRA_TESTS_6 \
    MResponseMatrixO6 Sum6_7 = Matrix.GetSumMatrixO6(1, 2, 3, 4, 5, 6); \
    Passed = EvaluateNear("GetSumMatrixO6()", "first six axes", "The order-6 sum projection preserves the representative first-bin content", Sum6_7.GetBinContent(0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_8 \
    RM_EXTRA_TESTS_7 \
    MResponseMatrixO7 Sum7_8 = Matrix.GetSumMatrixO7(1, 2, 3, 4, 5, 6, 7); \
    Passed = EvaluateNear("GetSumMatrixO7()", "first seven axes", "The order-7 sum projection preserves the representative first-bin content", Sum7_8.GetBinContent(0, 0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_9 \
    RM_EXTRA_TESTS_8 \
    MResponseMatrixO8 Sum8_9 = Matrix.GetSumMatrixO8(1, 2, 3, 4, 5, 6, 7, 8); \
    Passed = EvaluateNear("GetSumMatrixO8()", "first eight axes", "The order-8 sum projection preserves the representative first-bin content", Sum8_9.GetBinContent(0, 0, 0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_10 \
    RM_EXTRA_TESTS_9 \
    MResponseMatrixO9 Sum9_10 = Matrix.GetSumMatrixO9(1, 2, 3, 4, 5, 6, 7, 8, 9); \
    Passed = EvaluateNear("GetSumMatrixO9()", "first nine axes", "The order-9 sum projection preserves the representative first-bin content", Sum9_10.GetBinContent(0, 0, 0, 0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_11 \
    RM_EXTRA_TESTS_10 \
    MResponseMatrixO10 Sum10_11 = Matrix.GetSumMatrixO10(1, 2, 3, 4, 5, 6, 7, 8, 9, 10); \
    Passed = EvaluateNear("GetSumMatrixO10()", "first ten axes", "The order-10 sum projection preserves the representative first-bin content", Sum10_11.GetBinContent(0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_12 \
    RM_EXTRA_TESTS_11 \
    MResponseMatrixO11 Sum11_12 = Matrix.GetSumMatrixO11(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11); \
    Passed = EvaluateNear("GetSumMatrixO11()", "first eleven axes", "The order-11 sum projection preserves the representative first-bin content", Sum11_12.GetBinContent(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_13 \
    RM_EXTRA_TESTS_12 \
    MResponseMatrixO12 Sum12_13 = Matrix.GetSumMatrixO12(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12); \
    Passed = EvaluateNear("GetSumMatrixO12()", "first twelve axes", "The order-12 sum projection preserves the representative first-bin content", Sum12_13.GetBinContent(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_14 \
    RM_EXTRA_TESTS_13 \
    MResponseMatrixO13 Sum13_14 = Matrix.GetSumMatrixO13(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13); \
    Passed = EvaluateNear("GetSumMatrixO13()", "first thirteen axes", "The order-13 sum projection preserves the representative first-bin content", Sum13_14.GetBinContent(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_15 \
    RM_EXTRA_TESTS_14 \
    MResponseMatrixO14 Sum14_15 = Matrix.GetSumMatrixO14(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14); \
    Passed = EvaluateNear("GetSumMatrixO14()", "first fourteen axes", "The order-14 sum projection preserves the representative first-bin content", Sum14_15.GetBinContent(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_16 \
    RM_EXTRA_TESTS_15 \
    MResponseMatrixO15 Sum15_16 = Matrix.GetSumMatrixO15(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15); \
    Passed = EvaluateNear("GetSumMatrixO15()", "first fifteen axes", "The order-15 sum projection preserves the representative first-bin content", Sum15_16.GetBinContent(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define RM_EXTRA_TESTS_17 \
    RM_EXTRA_TESTS_16 \
    MResponseMatrixO16 Sum16_17 = Matrix.GetSumMatrixO16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16); \
    Passed = EvaluateNear("GetSumMatrixO16()", "first sixteen axes", "The order-16 sum projection preserves the representative first-bin content", Sum16_17.GetBinContent(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0), 6.0, 1e-6) && Passed;

#define DEFINE_UT_RESPONSE_MATRIX_ORDER(SUITE, MATRIX, ORDER, LOWERMATRIX) \
class SUITE : public MUnitTest \
{ \
public: \
  SUITE() : MUnitTest(#SUITE) {} \
  virtual ~SUITE() {} \
  virtual bool Run() \
  { \
    bool Passed = true; \
    Passed = EvaluateTrue("PrepareResponseMatrixTempDirectory()", "setup", "The temporary response-matrix directory can be created", PrepareResponseMatrixTempDirectory()) && Passed; \
    RM_DECLARE_AXES_##ORDER \
    vector<float> EmptyAxis; \
    vector<float> NonIncreasingAxis{0.0f, 1.0f, 1.0f}; \
    Passed = EvaluateTrue("SetAxis()", "empty first axis", "An empty first axis triggers the documented assertion path", RunAssertingUnitTest(MString("bin/") + #SUITE, "--assert-empty-first-axis")) && Passed; \
    Passed = EvaluateTrue("SetAxis()", "non-increasing last axis", "A non-increasing highest-order axis triggers the documented assertion path", RunAssertingUnitTest(MString("bin/") + #SUITE, "--assert-nonincreasing-last-axis")) && Passed; \
    MATRIX Default; \
    Passed = Evaluate("GetOrder()", "default constructor", "The default constructor sets the matrix order", Default.GetOrder(), static_cast<unsigned int>(ORDER)) && Passed; \
    Passed = Evaluate("GetNBins()", "default constructor", "The default constructor starts with zero bins", Default.GetNBins(), 0UL) && Passed; \
    Passed = Evaluate("GetAxisBins()", "default constructor", "The default constructor starts with zero bins on the highest-order axis", Default.GetAxisBins(ORDER), 0U) && Passed; \
    Default.SetAxis(RM_AXIS_ARGS_##ORDER); \
    Default.SetAxisNames(RM_AXIS_NAME_ARGS_##ORDER); \
    Passed = Evaluate("GetNBins()", "set axis", "Setting representative axes creates the expected number of bins", Default.GetNBins(), (1UL << ORDER)) && Passed; \
    Passed = Evaluate("GetAxisName()", "first axis", "The first axis name is stored correctly", Default.GetAxisName(1), MString("X1")) && Passed; \
    Passed = Evaluate("GetAxisName()", "last axis", "The last axis name is stored correctly", Default.GetAxisName(ORDER), MString("X" #ORDER)) && Passed; \
    Passed = EvaluateNear("GetAxis()", "first axis last edge", "GetAxis returns the expected edge vector for the first axis", Default.GetAxis(1).back(), 2.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetAxisContent()", "first axis first edge", "The first axis edge content is correct", Default.GetAxisContent(0, 1), 0.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetAxisContent()", "last axis last edge", "The last axis edge content is correct", Default.GetAxisContent(2, ORDER), 2.0, 1e-6) && Passed; \
    Passed = Evaluate("GetAxisBins()", "first axis", "The first axis has the expected number of bins", Default.GetAxisBins(1), 2U) && Passed; \
    Passed = Evaluate("GetAxisBins()", "last axis", "The last axis has the expected number of bins", Default.GetAxisBins(ORDER), 2U) && Passed; \
    Passed = EvaluateNear("GetAxisBinCenter()", "first axis", "The first bin center is correct", Default.GetAxisBinCenter(0, 1), 0.5, 1e-6) && Passed; \
    Passed = Evaluate("GetAxisBin()", "first axis", "The representative coordinate maps to the first bin", Default.GetAxisBin(0.5f, 1), 0U) && Passed; \
    Passed = EvaluateNear("GetAxisMinimum()", "first axis", "The first axis minimum is correct", Default.GetAxisMinimum(1), 0.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetAxisMaximum()", "last axis", "The last axis maximum is correct", Default.GetAxisMaximum(ORDER), 2.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetAxisLowEdge()", "last axis", "The last axis low edge is correct", Default.GetAxisLowEdge(0, ORDER), 0.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetAxisHighEdge()", "last axis", "The last axis high edge is correct", Default.GetAxisHighEdge(0, ORDER), 1.0, 1e-6) && Passed; \
    MATRIX DifferentAxis("Different", RM_AXIS_ARGS_DIFFERENT_##ORDER); \
    MATRIX Matrix("Representative", RM_AXIS_ARGS_##ORDER); \
    Matrix.SetAxisNames(RM_AXIS_NAME_ARGS_##ORDER); \
    Passed = Evaluate("operator==()", "same axes", "Matrices with identical axes compare equal", Matrix == Default, true) && Passed; \
    Passed = Evaluate("operator==()", "different axes", "Matrices with different axes compare unequal", Matrix == DifferentAxis, false) && Passed; \
    Matrix.SetBinContent(RM_ZERO_BINS_##ORDER, 5.0f); \
    Passed = EvaluateNear("GetBinContent()", "set bin content", "SetBinContent stores the representative first-bin value", Matrix.GetBinContent(RM_ZERO_BINS_##ORDER), 5.0, 1e-6) && Passed; \
    Matrix.Add(RM_VALUE_COORDS_##ORDER, 1.0f); \
    Passed = EvaluateNear("Add()", "representative add", "Add accumulates into the representative first bin", Matrix.GetBinContent(RM_ZERO_BINS_##ORDER), 6.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetBinArea()", "first bin", "The representative first bin has the expected volume", Matrix.GetBinArea(RM_ZERO_BINS_##ORDER), 1.0, 1e-6) && Passed; \
    Passed = EvaluateNear("Get()", "representative lookup", "Get returns the representative first-bin content", Matrix.Get(RM_VALUE_COORDS_##ORDER), 6.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetInterpolated()", "bin center", "Interpolation at the representative first-bin center returns the stored content", Matrix.GetInterpolated(RM_CENTER_COORDS_##ORDER, false), 6.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetMaximum()", "representative content", "GetMaximum returns the representative maximum", Matrix.GetMaximum(), 6.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetMinimum()", "representative content", "GetMinimum returns the representative minimum", Matrix.GetMinimum(), 0.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetSum()", "representative content", "GetSum returns the total representative content", Matrix.GetSum(), 6.0, 1e-6) && Passed; \
    LOWERMATRIX Lower(RM_AXIS_ARGS_PREV_##ORDER); \
    Lower.SetAxisNames(RM_AXIS_NAME_ARGS_PREV_##ORDER); \
    Lower.SetBinContent(RM_ZERO_BINS_PREV_##ORDER, 3.0f); \
    MATRIX WithSlice("WithSlice", RM_AXIS_ARGS_##ORDER); \
    WithSlice.SetAxisNames(RM_AXIS_NAME_ARGS_##ORDER); \
    WithSlice.SetMatrix(0, Lower); \
    Passed = EvaluateNear("SetMatrix()", "first slice", "SetMatrix installs the representative lower-order slice", WithSlice.GetBinContent(RM_ZERO_BINS_##ORDER), 3.0, 1e-6) && Passed; \
    MATRIX Other("Other", RM_AXIS_ARGS_##ORDER); \
    Other.SetBinContent(RM_ZERO_BINS_##ORDER, 2.0f); \
    MATRIX Combined = Matrix; \
    Combined += Other; \
    Passed = EvaluateNear("operator+=()", "matrix addition", "Matrix addition accumulates the representative first bin", Combined.GetBinContent(RM_ZERO_BINS_##ORDER), 8.0, 1e-6) && Passed; \
    Combined -= Other; \
    Passed = EvaluateNear("operator-=()", "matrix subtraction", "Matrix subtraction restores the representative first bin", Combined.GetBinContent(RM_ZERO_BINS_##ORDER), 6.0, 1e-6) && Passed; \
    MATRIX Divided = Matrix; \
    Divided /= Other; \
    Passed = EvaluateNear("operator/=()", "matrix division", "Matrix division divides the representative first bin", Divided.GetBinContent(RM_ZERO_BINS_##ORDER), 3.0, 1e-6) && Passed; \
    MATRIX Scalar("Scalar", RM_AXIS_ARGS_##ORDER); \
    Scalar += 1.0f; \
    Passed = EvaluateNear("operator+=(float)", "scalar addition", "Scalar addition affects every bin", Scalar.GetSum(), static_cast<double>(1UL << ORDER), 1e-6) && Passed; \
    Scalar *= 2.0f; \
    Passed = EvaluateNear("operator*=(float)", "scalar multiplication", "Scalar multiplication rescales every bin", Scalar.GetSum(), static_cast<double>(2UL << ORDER), 1e-6) && Passed; \
    MResponseMatrixO1 Sum1 = Matrix.GetSumMatrixO1(1); \
    Passed = EvaluateNear("GetSumMatrixO1()", "representative projection", "GetSumMatrixO1 preserves the representative first-bin content", Sum1.GetBinContent(0), 6.0, 1e-6) && Passed; \
    Passed = EvaluateNear("GetSumMatrixO1()", "representative projection empty bin", "GetSumMatrixO1 preserves the representative empty second bin", Sum1.GetBinContent(1), 0.0, 1e-6) && Passed; \
    RM_EXTRA_TESTS_##ORDER \
    MATRIX SumFull = Matrix.GetSumMatrixO##ORDER(RM_ORDER_INDICES_##ORDER); \
    Passed = EvaluateNear("GetSumMatrixO" #ORDER "()", "identity projection", "The full-order sum projection preserves the representative first-bin content", SumFull.GetBinContent(RM_ZERO_BINS_##ORDER), 6.0, 1e-6) && Passed; \
    MString FileName = MString("/tmp/UTResponseMatrix/") + #SUITE + ".rsp"; \
    Passed = Evaluate("Write()", "stream round trip", "Writing the representative matrix in stream mode succeeds", Matrix.Write(FileName, true), true) && Passed; \
    MATRIX ReadBack; \
    Passed = Evaluate("Read()", "stream round trip", "Reading the representative matrix written in stream mode succeeds", ReadBack.Read(FileName), true) && Passed; \
    Passed = EvaluateNear("GetBinContent()", "stream round trip", "The representative first-bin content survives a stream round trip", ReadBack.GetBinContent(RM_ZERO_BINS_##ORDER), 6.0, 1e-6) && Passed; \
    RM_HISTOGRAM_TESTS_##ORDER \
    { \
      bool WasBatch = gROOT->IsBatch(); \
      gROOT->SetBatch(true); \
      int BeforeCanvases = GetCanvasCount(); \
      DisableDefaultStreams(); \
      Matrix.Show(RM_HIST_ARGS_##ORDER, false); \
      EnableDefaultStreams(); \
      Passed = Evaluate("Show()", "representative display", "Show creates a ROOT canvas for the representative histogram", GetCanvasCount(), BeforeCanvases + 1) && Passed; \
      CleanupCanvases(BeforeCanvases); \
      gROOT->SetBatch(WasBatch); \
    } \
    DisableDefaultStreams(); \
    Passed = Evaluate("Write()", "non-stream mode", "Writing the representative matrix in non-stream mode is rejected explicitly", Matrix.Write(MString("/tmp/UTResponseMatrix/") + #SUITE + "_text.rsp", false), false) && Passed; \
    MFile File; \
    Passed = Evaluate("Open()", "non-stream read setup", "The representative non-stream response-matrix file can be created", File.Open(MString("/tmp/UTResponseMatrix/") + #SUITE + "_text_read.rsp", MFile::c_Write), true) && Passed; \
    File.Write(MString("# Response Matrix " #ORDER "\nVersion 1\n\nNM Representative\n\nOD " #ORDER "\n\nTS 0\n\nSA 0\n\nSM \n\nType ResponseMatrixO" #ORDER "\n")); \
    File.Close(); \
    MATRIX ReadBackText; \
    Passed = Evaluate("Read()", "non-stream mode", "Reading the representative matrix in non-stream mode is rejected explicitly", ReadBackText.Read(MString("/tmp/UTResponseMatrix/") + #SUITE + "_text_read.rsp"), false) && Passed; \
    EnableDefaultStreams(); \
    Summarize(); \
    return Passed; \
  } \
}; \
int main(int argc, char** argv) \
{ \
  if (argc == 2) { \
    RM_DECLARE_AXES_##ORDER \
    vector<float> EmptyAxis; \
    vector<float> NonIncreasingAxis{0.0f, 1.0f, 1.0f}; \
    MATRIX Matrix; \
    MString Argument = argv[1]; \
    if (Argument == "--assert-empty-first-axis") { \
      Matrix.SetAxis(RM_AXIS_ARGS_EMPTY_FIRST_##ORDER); \
      return 0; \
    } \
    if (Argument == "--assert-nonincreasing-last-axis") { \
      Matrix.SetAxis(RM_AXIS_ARGS_NONINCREASING_LAST_##ORDER); \
      return 0; \
    } \
  } \
  SUITE Test; \
  return Test.Run() == true ? 0 : 1; \
}

#endif
