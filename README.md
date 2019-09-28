# Deformation Model

Program is made for calculate normal section of a reinforced concrete element using nonlinear deformation model
Trough this application, one can easily draw or load from Excel file a geometry of a reinforced concrete section and then calculate it on acting forces.
As a result acting stresses and strains will be outputed for every divided part of a section. 
The result cuold be saved in an excel file.

### Installing

One can install the program from the automated installer from the releases section.

## User Interface

![User interface image](/Image/user_interface.png)
1 – main menu
2 – buttons of:
 - import / export geometry from excel
 - view navigation
3 – model window
4 – command line
5 – status line (current coordinates) 
6 – input of the concrete class section
7 – input of the concrete section geometry section
8 – input of the reinforcement class section
9 – input of the reinforcement bar position
10 –input of the forces in the section
11 – begin of the calculation button


## Running the program

### Input of the concrete section geometry

![Input geometry](/Image/Input1.png)

Input of the concrete section geometry options:
1 – graphically by mouse in the window:
   * “Line” – user polygonal figure:
     input points, press “c” in conclusion
   * “Rect” – rectangle:
     input two opposite vertexes 
2 – in the command line in numerical form, in case of the user figure press “c”
in conclusion
Select concrete and reinforcement steel class

### Import / export of the geometry from Excel

EImplemented the possibility of loading previously saved coordinates of the specific points from Excel,
And saving of the coordinates of the designed elements to Excel 
by means of the ActiveX technology

### Concrete section division 

![Divide the section](/Image/Divide.png)

1 – “Divide” starts the concrete section division
2 – input of the number of divisions along the X axis (10 by default),
   - input of the number of divisions along the Y axis (10 by default)
3 – the grid is shown in the window

### Input of position of the reinforcement bars

![Input reinforcement bars geometry](/Image/Reinforcement.png)

Select diameter for each reinforcement bar

Input of the coordinates of the reinforcement bars options:
1 – graphically by mouse in the window,       press “d” in conclusion
2 – in the command line in numerical form, press “d” in conclusion

### Input forces in the section and start the calculation

![Input forces and start the calculation](/Image/Forces.png)

1 – input of the axial force N, bending moment My along Y axis, bending moment Mx along X axis
2 – start of the calculation, in case of successful result the select options window will appear
3 – select options for result visualization
4 – select option for save result in excel
5 – apply the choice and close the window buttons

### Result visualization

![Visualisation of the result](/Image/Result.png)

1 – compressed zone of the section
2 – stretched zone of the section
3 – stress in the concrete element
4 – stress in the reinforcement bar

Depending of the selected option on the window could be displayed:
   - stresses in MPa,
   - stresses in kg/sm2,
   - strains multiplied by 1000,
   - areas of the elements in sm2

## Built With

* [Qt Creator](https://www.qt.io/) - Qt Creator
* [Inno Setup](http://www.jrsoftware.org/isinfo.php) - Inno Setup

## Authors

* **Denis Orlov** - [GitHub page](https://github.com/DOrlov74)

## License

This software is distributed under the terms and conditions of the GNU General Public License version 3.
![License](https://www.gnu.org/licenses/)


