//
//  ofxFontExtruder.cpp
//
//  Created by Rick Companje on 10/19/11.
//  Copyright (c) 2011 Globe4D. All rights reserved.
//

#include "ofxFontExtruder.h"

ofRectangle ofxFontExtruder::getStringBoundingBox(string s) {
    ofRectangle bounds = ofTrueTypeFont::getStringBoundingBox(s,0,0);

    int i=s.length();

    while (i>=0 && s[--i]==' ') {
        bounds.width += cps[(int)'p' - NUM_CHARACTER_TO_START].width;
    }

    if (bounds.x>0) {
        bounds.width+=bounds.x;
        bounds.x=0;
    }

    if (bounds.y<0) {
        bounds.y=0;   
    }
    
    return bounds;
}

ofRectangle ofxFontExtruder::getBounds() {
    return getStringBoundingBox(text);
}

ofVec2f ofxFontExtruder::getCharacterOffset(string s, int pos) {
    ofRectangle bounds = getStringBoundingBox(s.substr(0,pos));
    return ofVec2f(bounds.width, 0);
}

ofxMesh ofxFontExtruder::getMesh() {
    ofxMesh mesh;
    for (int i=0; i<text.length(); i++) {
        ofxMesh ch = getCharacterMesh(text[i]);
        ch.translate(getCharacterOffset(text,i));
        mesh.addMesh(ch);
    }
    return mesh;
}

ofxMesh ofxFontExtruder::getCharacterMesh(char letter) {
    if (letter==' ') return ofxMesh();
    
    ofVec3f zOffset(0,0,thickness);
        
    ofPath ch = getCharacterAsPoints(letter);
    
    vector<ofPolyline> outline = ch.getOutline();
    
    ofxMesh *tess = (ofxMesh*) &ch.getTessellation();
    ofxMesh top = *tess; //copy
    ofxMesh bottom = top; //copy
    ofxMesh sides;

    bottom.translate(zOffset); //extrude
                     
    //add top & bottom normals
    for (int i=0; i<top.getNumVertices(); i++) {
        top.addNormal(ofVec3f(0,0,-1));
        bottom.addNormal(ofVec3f(0,0,1));
    }
    
    //make side mesh by using outlines
    for (int j=0; j<outline.size(); j++) {
        
        int iMax = outline[j].getVertices().size();
        
        for (int i=0; i<iMax; i++) {
            
            ofVec3f a = outline[j].getVertices()[i];
            ofVec3f b = outline[j].getVertices()[i] + zOffset;
            ofVec3f c = outline[j].getVertices()[(i+1) % iMax] + zOffset;
            ofVec3f d = outline[j].getVertices()[(i+1) % iMax];
            
            sides.addFace(a,b,c,d);                          
        }
    }
    
    return ofxMesh(sides.addMesh(bottom.addMesh(top)));
}

void ofxFontExtruder::saveStl(string filename, bool isAscii) {
    
    float w = 100; //in mm
    float sc = w / getStringBoundingBox(text).width;
    
    ofxMesh mesh = getMesh();
    
    mesh.translate(ofVec3f(0,getStringBoundingBox(text).height,0));
    mesh.scale(1,-1);
    mesh.translate(-getStringBoundingBox(text).width/2, getStringBoundingBox(text).height/2,0);
    mesh.scale(sc,sc,sc);
    
    vector<ofVec3f>& vertices = mesh.getVertices();
    vector<ofIndexType>& indices = mesh.getIndices();
    vector<ofVec3f>& normals = mesh.getNormals();
    
    /// Export to STL
    //Note: if exporting to STL is not working check if the filename created in ofxSTL is ok. 
    //It may have a conflict with ofTrueTypeFont. /data should be ofxToDataPath in ofxSTL
    
    ofxSTLExporter stl;
	stl.beginModel();
    
    for (int i=0; i < indices.size()-2; i+=3) {
        stl.addTriangle(vertices[indices[i+2]], vertices[indices[i+1]], vertices[indices[i]], normals[indices[i]]);
    }
	
    stl.useASCIIFormat(isAscii);
    stl.saveModel(filename);
}


void ofxFontExtruder::saveGCode(string filename, string headerFile, string footerFile) {
    
    ofxGCode gcode;
    
	// Skein variables
	float feedrate = 1800;		// in mm/s- this is fixed.
	float flowrate = 1.573;		// in RPM
	float layerHeight = .3;		// in mm
	float zOffset = .3;         // in mm - added to all z coordinates.
    float maxSize = 100;        // in mm - size of build platform
    
    //tmnp
    int layers = 100;
    
    // Scaling pixels to mm and center
	float scale =  maxSize / getStringBoundingBox(text).width;
	float xShift = -getStringBoundingBox(text).width/2;
	float yShift = getStringBoundingBox(text).height/2;
    
    if (ofFile(headerFile).exists()) gcode.insert(headerFile);
    
    vector<ofTTFCharacter> characters = getStringAsPoints(text);
    
    for (int layer=0; layer<layers; layer++) {
        
        for (int ch=0; ch<characters.size(); ch++) {
            
            gcode.addComment("character: " + ofToString(text[ch]));
            
            vector<ofPolyline> contours = characters[ch].getOutline();
            
            for (int contour=0; contour<contours.size(); contour++) {
                
                gcode.addComment("contour: " + ofToString(contour));
                
                contours[contour].simplify();
                
                vector<ofVec3f> points = contours[contour].getVertices();
                
                // Move to each point in the contour
                for(int i=0; i<points.size(); i++) {
                    
                    if (i==1) gcode.addCommand("M101", "start the extruder");
                    
                    gcode.addCommandWithParams("G1 X%03f Y%03f Z%03f F%03f",
                                               scale * (points[i].x + xShift),
                                               scale * -(points[i].y + yShift),
                                               layer * layerHeight + zOffset,
                                               feedrate);
                }
                
                gcode.addCommand("M103", "stop the extruder");
            }   
        }
    }
    
    if (ofFile(footerFile).exists()) gcode.insert(footerFile);
    
    gcode.save(filename);
}

