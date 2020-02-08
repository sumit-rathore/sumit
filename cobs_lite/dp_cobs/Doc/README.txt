The User's Guide is Hobbes User's Guide.pdf. The TeX file used to generate said
PDF file is Hobbes.tex. The image required to be shown in the PDF is
interpreterdiff.pdf. Don't delete that last file. Heck, don't delete any of
them at all.

If you want to update the User's Guide by yourself, you'll have to do a bit of
work. I don't believe there's anything on Shamrock that would allow you to
compile TeX to PDF, so you might have to take the time-consuming method (as I
did) if you want to generate a PDF file. I assume you are running Windows.

1. Get MiKTeX:
http://miktex.org/2.9/setup
(or whatever the most recent version is)

2. Get Texmaker:
http://www.xm1math.net/texmaker/download.html#windows

3. Open Texmaker and the .tex file. Then open Options > Configure Texmaker,
select "Quick Build" on the left, and select "PdfLaTeX + View PDF" under "Quick
Build Command".

4. Select "Commands" on the left in the same window, and make sure all the
commands are populated. For "PdfLaTeX", you should have something like

pdflatex -interaction=nonstopmode %.tex

and for "Pdf Viewer", make sure you have whatever PDF viewer you like; I have
Adobe Reader so my command line there is

"C:/Program Files/Adobe/Reader 10.0/Reader/AcroRd32.exe" %.pdf

and I've selected "Built-in Viewer".

5. Run Quick Build. Hitting F1 should compile the TeX file, and open Texmaker's
PDF viewer (or your custom PDF viewer) displaying the User's Guide.


Note: if at any point you don't see a table of contents, or you've added a new
section but it doesn't show up, run Quick Build again. The first run would have
created, but not have actually used, a .toc file. Running it again uses the
generated .toc file, and you should then see a nice table of contents with
links to every section.
