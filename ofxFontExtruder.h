//
//  ofxFontExtruder.h
//
//  Created by Rick Companje on 10/19/11.
//  Copyright (c) 2011 Globe4D. All rights reserved.
//

#pragma once

#include "ofMain.h"
#include "ofxMesh.h"
#include "ofxSTL.h"
#include "ofxGCode.h"

class ofxFontExtruder : public ofTrueTypeFont {
public:
    
    ofxFontExtruder() {
        setText("OF");
        setThickness(100);
    }
    
    ofRectangle getBounds();    
    ofxMesh getMesh();

    void setText(string text) { this->text = text; }
    string &getText() { return text; }
    
    void setThickness(float thickness) { this->thickness = thickness; }
    float &getThickness() { return thickness; }
    
    void saveGCode(string filename, string headerFile="start.gcode", string footerFile="end.gcode");
    void saveStl(string filename, bool isAscii=true);

protected:
    ofxMesh getCharacterMesh(char letter);
    ofRectangle getStringBoundingBox(string s);
    ofVec2f getCharacterOffset(string s, int index);

    string text;
    float thickness;
};

