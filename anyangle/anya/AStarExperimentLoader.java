package pgraph.anya.experiments;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by Dindar on 22.8.2014.
 */
public class AStarExperimentLoader {

    public List<ExperimentInterface> loadExperiments(String mapFile) throws IOException {

        FileReader fstream = null;
        fstream = new FileReader(mapFile);
        BufferedReader in= new BufferedReader(fstream);

        List<ExperimentInterface> expList= new ArrayList<>();

        String versionLine = in.readLine();
        String expLine = in.readLine();
        ExperimentInterface exp = null;
        int experimentCounter = 0;
        while ( expLine  != null)
        {
            String expTokens[] = expLine.split("\t");
            if (expTokens.length <9)
            {
                expTokens = expLine.split(" ");
                if (expTokens.length <9)
                {
                    expLine = in.readLine();
                    continue;
                }
            }
            experimentCounter++;

            try {
                exp = new AStarExperiment(experimentCounter+"",expTokens[1],
                                        Integer.parseInt(expTokens[2]),
                                        Integer.parseInt(expTokens[3]),
                                        Integer.parseInt(expTokens[4]),
                                        Integer.parseInt(expTokens[5]),
                                        Integer.parseInt(expTokens[6]),
                                        Integer.parseInt(expTokens[7]),
                                        Double.parseDouble(expTokens[8]));

            } catch (NumberFormatException e) {
                e.printStackTrace();
                expLine = in.readLine();
                continue;
            }
            expList.add(exp);

            expLine = in.readLine();
        }


        return expList;
    }
}
