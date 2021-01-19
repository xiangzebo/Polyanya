package pgraph.anya.experiments;

/**
 * Created by Dindar on 22.8.2014.
 */
public class AStarExperiment implements ExperimentInterface {
    private String mapFile;

    private int xSize;
    private int ySize;
    private int startX;
    private int startY;
    private int endX;
    private int endY;
    private double upperBound;




    public void setTitle(String title) {
        this.title = title;
    }

    private String title;

    public AStarExperiment(String mapFile, int xSize, int ySize, int startX, int startY, int endX, int endY) {
        this.mapFile = mapFile;
        this.xSize = xSize;
        this.ySize = ySize;
        this.startX = startX;
        this.startY = startY;
        this.endX = endX;
        this.endY = endY;
    }

    public AStarExperiment(String mapFile, int xSize, int ySize, int startX, int startY, int endX, int endY, double upperBound) {
        this.mapFile = mapFile;
        this.xSize = xSize;
        this.ySize = ySize;
        this.startX = startX;
        this.startY = startY;
        this.endX = endX;
        this.endY = endY;
        this.upperBound = upperBound;
    }

    public AStarExperiment(String title, String mapFile, int xSize, int ySize, int startX, int startY, int endX, int endY, double upperBound) {
        this.title=title;
        this.mapFile = mapFile;
        this.xSize = xSize;
        this.ySize = ySize;
        this.startX = startX;
        this.startY = startY;
        this.endX = endX;
        this.endY = endY;
        this.upperBound = upperBound;
    }

    public void setUpperBound(double upperBound) {
        this.upperBound = upperBound;
    }

    public void setMapFile(String mapFile) {
        this.mapFile = mapFile;
    }

    public void setStartX(int startX) {
        this.startX = startX;
    }

    public void setStartY(int startY) {
        this.startY = startY;
    }

    public void setEndX(int endX) {
        this.endX = endX;
    }

    public void setEndY(int endY) {
        this.endY = endY;
    }

    @Override
    public String getMapFile() {
        return mapFile;
    }

    @Override
    public int getXSize() {
        return xSize;
    }

    @Override
    public int getYSize() {
        return ySize;
    }

    @Override
    public int getStartX() {
        return  startX;
    }

    @Override
    public int getStartY() {
        return  startY;
    }

    @Override
    public int getEndX() {
        return  endX;
    }

    @Override
    public int getEndY() {
        return  endY;
    }

    @Override
    public double getUpperBound() {
        return upperBound;
    }


}
