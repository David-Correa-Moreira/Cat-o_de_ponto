<?php

namespace App\Services;

use GuzzleHttp\Client;
use App\Models\U;
use Carbon\Carbon;
use Illuminate\Support\Str;

//primeira classe da vida
class PontoService
{
    //cria um método estático
    public static function verificaPresenca()
    {
        $usarios = U::where('s', true)->get();
        

        //dd(Carbon::today());
       
        foreach ($usarios as $usario) {
            $register = $usario->r()->whereDate('created_at', Carbon::today())->first();
            if (!$register) {
                //dump("Vai enviar mensagen : {$usario->u}");
                self::enviarMensagem($usario->u);
            }
                
        }
    }

    /**
     * to: Número de telefone do contato que irá receber sua mensagem. 
     * Atenção: Sempre incluir o código do pais,ddd,número e no final 
     * adicionar a tag @c.us caso a mensagem seja enviada para um 
     * contato ou a tag @g.us se a mensagem for enviada pra um grupo.
     */

    public static function enviarMensagem($usuario)
    {
        $id = (string) Str::uuid();
        $telefone =  '5592995035898@c.us';
        $token = 'vc6CnpxldiiK7M6o5iO1wd9CTn1oCE8r3TCA';
        $endpoint = 'https://api.winzap.com.br/send/';

        // Create a client with a base URI
        $client = new Client();


        /* token: Código de identificação da conta
        cmd: Tipo da mensagem a ser enviada, neste caso: chat
        id: Para que você capture atualizações de status pelo webhook referente a mensagem enviada, 
        é necessário que você identifique cada mensagem enviada pela API com um ID único, neste caso 
        (randômico)
        to: Número de telefone do contato que irá receber sua mensagem. Atenção: Sempre incluir o código do pais,ddd,número e no final adicionar a tag @c.us caso a mensagem seja enviada para um contato ou a tag @g.us se a mensagem for enviada pra um grupo.
        msg: Conteudo da mensagem a ser enviada
         */
        $response = $client->request('POST', $endpoint, [
            'form_params' => [
                'token' => $token,
                'cmd'   => 'chat',
                'id'    => $id,
                'to'    => $telefone,
                'msg'   => "*CENTRAL DE CONTROLE INFORMA*\n\nO ponto do o usuario *{$usuario}* ainda não foi registrado!!"
            ]
        ]);

        //dd($response->getStatusCode(), (string) $response->getBody()); // 200)
    }


    /*********** envio agrupado de mensagens******* */
    public static function verificaPresencaGrupo()
    {
        $usarios = U::where('s', true)->get();
        $nomes = null;

        $diaAtual = Carbon::today();
        
        // if ( ($diaAtual->dayOfWeek !=  6) && ($diaAtual->dayOfWeek !=  0) ) {
        if (!in_array($diaAtual->dayOfWeek, [0, 6])) {
            
            foreach ($usarios as $usario) {
                $register = $usario->r()->whereDate('created_at', Carbon::today())->first();
                if (!$register) {
                    //dump("Vai enviar mensagen : {$usario->u}");
                    $nomes .= "\n*{$usario->u}*";
                }        
            }
            //dd( $nomes);
            if ($nomes) {
                self::enviarMensagemGrupo($nomes);
            }
        }
    }

    /**
     * to: Número de telefone do contato que irá receber sua mensagem. 
     * Atenção: Sempre incluir o código do pais,ddd,número e no final 
     * adicionar a tag @c.us caso a mensagem seja enviada para um 
     * contato ou a tag @g.us se a mensagem for enviada pra um grupo.
     */

    public static function enviarMensagemGrupo($nomes)
    {
        $id = (string) Str::uuid();
        $telefone =  '5592995035898@c.us';
        $token = 'vc6CnpxldiiK7M6o5iO1wd9CTn1oCE8r3TCA';
        $endpoint = 'https://api.winzap.com.br/send/';
        $msg = "*CENTRAL DE CONTROLE INFORMA*\n\nOs usuários: {$nomes}\nainda não tiveram o ponto registrado!!";

        // Create a client with a base URI
        $client = new Client();


        /* token: Código de identificação da conta
        cmd: Tipo da mensagem a ser enviada, neste caso: chat
        id: Para que você capture atualizações de status pelo webhook referente a mensagem enviada, 
        é necessário que você identifique cada mensagem enviada pela API com um ID único, neste caso 
        (randômico)
        to: Número de telefone do contato que irá receber sua mensagem. Atenção: Sempre incluir o código do pais,ddd,número e no final adicionar a tag @c.us caso a mensagem seja enviada para um contato ou a tag @g.us se a mensagem for enviada pra um grupo.
        msg: Conteudo da mensagem a ser enviada
         */
        $response = $client->request('POST', $endpoint, [
            'form_params' => [
                'token' => $token,
                'cmd'   => 'chat',
                'id'    => $id,
                'to'    => $telefone,
                'msg'   => $msg
            ]
        ]);

        //dd($response->getStatusCode(), (string) $response->getBody()); // 200)
    }


}
