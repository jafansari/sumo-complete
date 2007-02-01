/****************************************************************************/
/// @file    Image.h
/// @author  unknown_author
/// @date    Fri, 29.04.2005
/// @version $Id: $
///
//
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// copyright : (C) 2001-2007
//  by DLR (http://www.dlr.de/) and ZAIK (http://www.zaik.uni-koeln.de/AFS)
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
#ifndef Image_h
#define Image_h
// ===========================================================================
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <fx.h>
#include "gui/GUIApplicationWindow.h"
#include "Point.h"
#include "Graph.h"

// ===========================================================================
// class definitions
// ===========================================================================
/**
 *
 */
class Image
{

private:

    //Zwei Pointer auf FXImage-Objekte, �ber die die Bildbearbeitung abl�uft
    FXImage *m_img;
    FXImage *m_transimg;

public:

    //Konstruktoren (1.leer / 2. Zwei Pointer auf ein FXImage-Objekt und ein FXAPP-Objekt)
    Image();

    Image(FXImage *img,FXApp *a);

    //Erstellt eine Kopie des FXImage-Objekts
    void Copy(FXImage *source,FXImage *destination);

    // ???
    int CountTransitions(int i, int j);

    //Holt das FXImage-Objekt
    FXImage* GetFXImage();

    /*
    Erzeugt aus dem Ausgangsbild ein Schwarzwei�bild,
    in denen die f�r uns relevanten Farben in schwarz
    und alle anderen Farben in wei� gef�rbt werden
    */
    void ExtractStreets();

    /*
    F�hrt eine Erosion durch, d.h. eine Verd�nnung
    der Schw�rzen Fl�chen des Bildes
    */
    void Erode(ConfigDialog* myDialog);

    /*
    Invers zur Erosion, d.h. hier werden die schwarzen
    Fl�chen verbreitert
    */
    void Dilate(ConfigDialog* myDialog);

    /*
    F�hrt erst eine Erosion, dann eine Dilatation durch
    */
    void Opening(ConfigDialog* myDialog);

    /*
    Invers zu Opening(Dilatation->Erosion)
    */
    void Closing(ConfigDialog* myDialog);

    /*
    F�rbt wei�e Pixel schwarz, die mehr als f�nf schwarze
    Nachbarpixel haben
    */
    void CloseGaps();

    /*
    Erzeugt ein Skelett, also Pixelketten aus den
    schwarzen Fl�chen des Ausgangsbildes
    */
    void CreateSkeleton();

    /*
    Minimiert das Skelett, so da� jedes jetzt
    entfernte schwarze Pixel die Kette sprengt
    */
    void RarifySkeleton();

    /*
    Entfernt scharze Elemente(Fragmente), die
    nicht zu Stra�en geh�ren
    */
    void EraseStains(ConfigDialog* myDialog);

    Graph* Tracking(Graph* gr,ConfigDialog* myDialog);
    /*
       Erzeugt einen Graphen aus dem minimalen Skelett
       */
    //Graph Tracking(Graph gr, ConfigDialog* myDialog);

    /*
    Hilfsmethode f�r die Grapherzeugung (Tracking)
    */
    Graph Pixel_Counter(int i,int j,int i_pre, int j_pre,int count,Graph gr,Vertex* temp, int value);

    /*
    Malt die Knoten des Graphen ins Bild
    */
    void DrawVArray(Graph gr);

    /*
    Malt die Kanten des Graphen ins Bild
    */
    void DrawEArray(Graph gr);

    /*
    Malt den Graph ins Bild
    */
    void DrawGraph(Graph gr);

    /*
    Erzeugt einen Punkt auf dem Bild
    */
    void CreatePoint(Point p, bool cross);

    /*
    Erzeugt eine Linie auf dem Bild (anhand zweier Punkte)
    */
    void DrawLine(Point p1, Point p2);

    /*
    Erzeugt eine Linie auf dem Bild (anhand der vier Koordinaten zweier Punkte)
    */
    void DrawLine(FXint i,FXint j,FXint k,FXint l);

    /*
    Erzeugt ein leeres Image
    */
    void EmptyImage();

    //Hat ein Pixel schwarze Nachbarn?
    Vertex* black_neighbour(Vertex* start);
    Vertex* black_neighbour(Vertex* lauf, Vertex* altlauf);

    //Ablaufen der schwarzen Pixel bis Counter oder bis zum n�chsten Knoten
    Graph* edgerun(Vertex* start, Graph* gr, Graph* helpgraph, int value);

};


#endif

/****************************************************************************/

