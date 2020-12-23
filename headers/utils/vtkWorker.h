//
// Created by GuVictory on 23.12.2020.
//

#ifndef INC_3D_CURRENTLINEVISUALIZATION_VTKWORKER_H
#define INC_3D_CURRENTLINEVISUALIZATION_VTKWORKER_H

#include <vtkSmartPointer.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>

/*
	Загружает файл с заданным именем файла и возвращает ссылку на сгенерированный объект vtkStructuredPoints
	@param имя файла
	@return VtkSmartPointer на объект vtkStructuredPoints
*/
vtkSmartPointer<vtkStructuredPoints> loadStructuredPointsFile(char* filename)
{
    // Определяем reader и загружаем файл
    vtkSmartPointer<vtkStructuredPointsReader> reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
    reader->SetFileName(filename);
    reader->Update();

    // Возвращаем созданный объект
    return reader->GetOutput();
}

#endif //INC_3D_CURRENTLINEVISUALIZATION_VTKWORKER_H
