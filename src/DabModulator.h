/*
   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
   Her Majesty the Queen in Right of Canada (Communications Research
   Center Canada)

   Copyright (C) 2015
   Matthias P. Braendli, matthias.braendli@mpb.li

    http://opendigitalradio.org
 */
/*
   This file is part of ODR-DabMod.

   ODR-DabMod is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   ODR-DabMod is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with ODR-DabMod.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DAB_MODULATOR_H
#define DAB_MODULATOR_H

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include <sys/types.h>
#include <string>
#include <boost/shared_ptr.hpp>

#include "ModCodec.h"
#include "EtiReader.h"
#include "Flowgraph.h"
#include "GainControl.h"
#include "OutputMemory.h"
#include "RemoteControl.h"
#include "Log.h"


class DabModulator : public ModCodec
{
public:
    DabModulator(
            struct modulator_offset_config& modconf,
            RemoteControllers* rcs,
            Logger& logger,
            unsigned outputRate = 2048000, unsigned clockRate = 0,
            unsigned dabMode = 0, GainMode gainMode = GAIN_VAR,
            float digGain = 1.0, float normalise = 1.0,
            std::string filterTapsFilename = "");
    DabModulator(const DabModulator& copy);
    virtual ~DabModulator();

    int process(Buffer* const dataIn, Buffer* dataOut);
    const char* name() { return "DabModulator"; }

    /* Required to get the timestamp */
    EtiReader* getEtiReader() { return &myEtiReader; }

protected:
    Logger& myLogger;

    void setMode(unsigned mode);

    unsigned myOutputRate;
    unsigned myClockRate;
    unsigned myDabMode;
    GainMode myGainMode;
    float myDigGain;
    float myNormalise;
    EtiReader myEtiReader;
    Flowgraph* myFlowgraph;
    OutputMemory* myOutput;
    std::string myFilterTapsFilename;
    RemoteControllers* myRCs;

    size_t myNbSymbols;
    size_t myNbCarriers;
    size_t mySpacing;
    size_t myNullSize;
    size_t mySymSize;
    size_t myFicSizeOut;
    size_t myFicSizeIn;
};

#endif // DAB_MODULATOR_H

