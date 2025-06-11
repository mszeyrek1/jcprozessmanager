package com.example.demo;

import javafx.application.Application;
import javafx.collections.FXCollections;
import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.TableView;
import javafx.scene.layout.VBox;
import javafx.scene.control.TableColumn;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.stage.Stage;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Arrays;
import java.util.List;
import com.fasterxml.jackson.databind.ObjectMapper;
import static javafx.scene.control.TableView.CONSTRAINED_RESIZE_POLICY;

public class Main extends Application {
    @Override
    public void start(Stage stage) {
        TableView<prozessinfo> table = new TableView<>();
        TableColumn<prozessinfo,String> pid = new TableColumn<>("PID");
        pid.setCellValueFactory(new PropertyValueFactory<>("pid"));

        TableColumn<prozessinfo,String> ppid = new TableColumn<>("PPID");
        ppid.setCellValueFactory(new  PropertyValueFactory<>("ppid"));

        TableColumn<prozessinfo,String> name = new TableColumn<>("Name");
        name.setCellValueFactory(new PropertyValueFactory<>("name"));

        TableColumn<prozessinfo,String> state = new TableColumn<>("State");
        state.setCellValueFactory(new PropertyValueFactory<>("state"));

        TableColumn<prozessinfo,String> vmsize = new TableColumn<>("VMsize");
        vmsize.setCellValueFactory(new PropertyValueFactory<>("vmsize"));

        TableColumn<prozessinfo,String> rss = new TableColumn<>("RSS");
        rss.setCellValueFactory(new PropertyValueFactory<>("rss"));

        TableColumn<prozessinfo,String> cputime = new TableColumn<>("Cputime");
        cputime.setCellValueFactory(new PropertyValueFactory<>("cputime"));

        table.getColumns().addAll(pid,ppid,name,state,vmsize,rss,cputime);
        table.setColumnResizePolicy(CONSTRAINED_RESIZE_POLICY);
        try {
            String jsonOutput = runProcessInfoCommand();
            System.out.println("JSON Output:\n" +  jsonOutput);
            List<prozessinfo> processList = parseJson(jsonOutput);
            table.setItems(FXCollections.observableArrayList(processList));
        } catch (IOException e) {
            e.printStackTrace();
        }
        VBox root = new VBox();
        root.getChildren().addAll(table);
        root.setSpacing(10);
        root.setPadding(new Insets(10));

        Scene scene = new Scene(root, 800, 600);
        stage.setScene(scene);
        stage.setTitle("Prozessübersicht");
        stage.show();
    }
    public static String runProcessInfoCommand() throws IOException {
        ProcessBuilder pb = new ProcessBuilder("./prozessinfo");
        pb.directory(new File("home/Desktop/jcprozessmanager/backend-c/src"));  // Pfad zu prozessinfo anpassen
        pb.redirectErrorStream(true);
        Process process = pb.start();

        BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
        StringBuilder output = new StringBuilder();
        String line;
        while ((line = reader.readLine()) != null) {
            output.append(line).append("\n");
        }
        return output.toString();
    }

    public static List<prozessinfo> parseJson(String json) throws IOException {
        ObjectMapper mapper = new ObjectMapper();
        return Arrays.asList(mapper.readValue(json, prozessinfo[].class));
    }



    public static void main(String[] args) {
        launch();
    }
}
