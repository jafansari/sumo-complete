// Vertex.h: Schnittstelle f�r die Klasse Vertex.
//
//////////////////////////////////////////////////////////////////////
#ifndef _VERTEX_H_
#define _VERTEX_H_

#include <vector>
#include "Edge.h"
#include "math.h"
using namespace std;

class Vertex{

private:

    //Koordinaten, Grad, Eingangsgrad und Ausgangsgrad des Knoten
    int x,y,id,degree,inDegree,outDegree;

    //Gau�-Kr�ger Koordinaten und GPS-Koordinaten(Longitude,Latitude)
    double gkx,gky,lon,lat;

    // Dynamisches Array, welches alle Pointer auf Nachfolgerknoten enth�lt
    vector <Vertex*> nachfolger;
    // Dynamisches Array, welches alle Pointer auf Vorgaengerknoten enth�lt
    vector <Vertex*> vorgaenger;
    
public:

    //Konstruktoren (Koordinaten)
    Vertex(int i, int j);

    //F�ge einen Pointer auf einen Knoten in das Nachfolgerarray ein
    void AddNachfolger(Vertex* v);
    //F�ge einen Pointer auf einen Knoten in das Vorgaengerarray ein
    void AddVorgaenger(Vertex* v);
        
    //L�sche den Pointer an Stelle k aus dem Nachfolgerarray
    void DelNachfolger(int k);
    //L�sche den Pointer an Stelle k aus dem Vorgaengerarray
    void DelVorgaenger(int k);

    //Hole den Pointer an Stelle i in dem Nachfolgerarray
    Vertex* GetNachfolgeVertex(int i);
    //Hole den Pointer an Stelle i in dem Vorgaengerarray
    Vertex* GetVorgaengerVertex(int i);
    
    //L�sche die Arrays(1.Nachfolgerarray / 2.Vorgaengerarray / 3.Array der Inzidenten Kanten)
    void DelNachfolgeArray();
    void DelVorgaengerArray();
    
    //Liefert die Anzahl der Elemente im Array zur�ck
    size_t GetNachfolger();
    size_t GetVorgaenger();
    
    //Liefert die Koordinaten des Knoten
    int GetX();
    int GetY();

    //Liefert den Index des Knotens
    int GetId();

    //Liefert den Grad (Gesamt/Eingangsgrad/Ausgangsgrad) des Knotens
    int GetDegree();
    int GetInDegree();
    int GetOutDegree();

    //Setzt die Koordinaten des Knoten
    void SetX(int i);
    void SetY(int j);

    //Setzt den Index des Knotens
    void SetId(int n);

    //Erh�hung der Grade des Knotens
    void InkrementDegree();
    void InkrementInDegree();
    void InkrementOutDegree();

    //Verringerung der Grade des Knotens
    void DekrementDegree();
    void DekrementInDegree();
    void DekrementOutDegree();


    //Wandelt die Koordinaten des Knotens in GPS-Koordinaten um
    void px2gps(int i, int j);
    //Hole den Longitude-Wert der GPS-Koordinaten
    double GetGPSLon();
    //Hole den Latitude-Wert der GPS-Koordinaten
    double GetGPSLat();

};

#endif
    //_VERTEX_H