/*

  Author: Lubosz Sarnecki
  2013

*/

#ifndef GESDEMO_H
#define GESDEMO_H

#include <unistd.h>
#include <stdio.h>
#include <gst/gst.h>
#include <ges/ges.h>

GESTimeline *transitionTL (void);
GESTimeline *effectTL (void);
GESTimeline *testTL (void);
GESTimeline *minuteTL (void);
GESTimeline *imageTL (void);
GESTimeline *oneTL (void);
GESTimeline *musicTL (void);
GESTimeline *hdTL (void);

#endif // GESDEMO_H
