/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkRecord.h"
#include "SkRecorder.h"
#include "SkRecords.h"

#include "SkEmptyShader.h"

#define COUNT(T) + 1
static const int kRecordTypes = SK_RECORD_TYPES(COUNT);
#undef COUNT

// Tallies the types of commands it sees into a histogram.
class Tally {
public:
    Tally() { sk_bzero(&fHistogram, sizeof(fHistogram)); }

    template <typename T>
    void operator()(const T&) { ++fHistogram[T::kType]; }

    template <typename T>
    int count() const { return fHistogram[T::kType]; }

private:
    int fHistogram[kRecordTypes];
};

DEF_TEST(Recorder, r) {
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, 1920, 1080);

    recorder.drawRect(SkRect::MakeWH(10, 10), SkPaint());

    Tally tally;
    record.visit(tally);

    REPORTER_ASSERT(r, 1 == tally.count<SkRecords::DrawRect>());
}

// Regression test for leaking refs held by optional arguments.
DEF_TEST(Recorder_RefLeaking, r) {
    // We use SaveLayer to test:
    //   - its SkRect argument is optional and SkRect is POD.  Just testing that that works.
    //   - its SkPaint argument is optional and SkPaint is not POD.  The bug was here.

    SkRect bounds;
    SkPaint paint;
    paint.setShader(SkNEW(SkEmptyShader))->unref();

    REPORTER_ASSERT(r, paint.getShader()->unique());
    {
        SkRecord record;
        SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record, 1920, 1080);
        recorder.saveLayer(&bounds, &paint);
        REPORTER_ASSERT(r, !paint.getShader()->unique());
    }
    REPORTER_ASSERT(r, paint.getShader()->unique());
}