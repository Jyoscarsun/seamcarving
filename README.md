# Seam Carving

## Project Overview
This project implements the **seam carving algorithm**, a content-aware image resizing technique that reduces the dimensions of an image while preserving its most important features. Unlike traditional resizing methods like cropping or scaling, seam carving identifies and removes paths of pixels (seams) that contribute the least to the image's overall visual content.

Seam carving is widely used in computer graphics applications, including Adobe Photoshop, to intelligently resize images while maintaining aspect ratios and preserving key objects.

## Key Features
- **Dual-Gradient Energy Calculation**: Computes the energy of each pixel to determine its importance in the image.
- **Dynamic Programming for Seam Identification**: Finds vertical or horizontal seams with the minimum total energy using an optimized dynamic programming approach.
- **Seam Removal**: Removes the identified seams to reduce the image's size.
- **Iterative Resizing**: Supports repeated seam removal to achieve desired dimensions.
